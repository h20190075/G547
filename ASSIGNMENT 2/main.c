#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/usb.h>
#include<linux/slab.h>



# define HZ  CONFIG_HZ   

#define Hewlett_Packard_VID  0x03f0
#define Hewlett_Packard_PID 0x1a40

#define RETRY_MAX                     5
#define READ_CAPACITY_LENGTH          64
#define USB_ENDPOINT_IN          0x80
#define BOMS_RESET_REQ_TYPE           0x21
#define LIBUSB_REQUEST_TYPE_CLASS    (0x01<<5)
#define LIBUSB_RECIPIENT_INTERFACE   0x01
#define be_to_int32(buf) (((buf)[0]<<24)|((buf)[1]<<16)|((buf)[2]<<8)|(buf)[3])
#define BOMS_RESET                    0xFF
#define BOMS_GET_MAX_LUN              0xFE
#define BOMS_GET_MAX_LUN_REQ_TYPE     0xA1


struct command_block_wrapper {
	uint8_t dCBWSignature[4];
	uint32_t dCBWTag;
	uint32_t dCBWDataTransferLength;
	uint8_t bmCBWFlags;
	uint8_t bCBWLUN;
	uint8_t bCBWCBLength;
	uint8_t CBWCB[16];
};

// Command Status Wrapper (CSW)
struct command_status_wrapper {
	uint8_t dCSWSignature[4];
	uint32_t dCSWTag;
	uint32_t dCSWDataResidue;
	uint8_t bCSWStatus;
};
static uint8_t cdb_length[256] = {
//	 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  0
	06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,06,  //  1
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  2
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  3
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  4
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,  //  5
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  6
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  7
	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  8
	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,  //  9
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  A
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,  //  B
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  C
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  D
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  E
	00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,  //  F
};


struct usbdev_private
{
	struct usb_device *udev;
	unsigned char class;
	unsigned char subclass;
	unsigned char protocol;
	unsigned char ep_in;
	unsigned char ep_out;
};

struct usbdev_private *p_usbdev_info;

static void usbdev_disconnect(struct usb_interface *interface)
{
	printk(KERN_INFO "USBDEV Device Removed\n");
	return;
}

static struct usb_device_id usbdev_table [] = {
	{USB_DEVICE(Hewlett_Packard_VID, Hewlett_Packard_PID)},
	
	{} /*terminating entry*/	
};


static int get_mass_storage_status(struct usb_device *udev, uint8_t endpoint, uint32_t expected_tag)
{	struct command_status_wrapper *csw=kmalloc(sizeof(struct command_status_wrapper),GFP_KERNEL);
	int r,size;
	r=usb_bulk_msg(udev,usb_rcvbulkpipe(udev,endpoint),(void *)csw,13, &size, 1000);
	if(r<0)
		printk("status cant be fetched");
}
static int send_mass_storage_command(struct usb_device *udev, uint8_t endpoint, uint8_t lun,uint8_t *cdb, uint8_t direction, int data_length, uint32_t *ret_tag)
{
	struct command_block_wrapper *cbw=kmalloc(sizeof(struct command_block_wrapper),GFP_KERNEL);
	
	static uint32_t tag = 1;
	int i=0, r,size;
	uint8_t cdb_len;	
        cdb_len = cdb_length[cdb[0]];
	memset(cbw, 0, sizeof(*cbw));
	cbw->dCBWSignature[0] = 'U';
	cbw->dCBWSignature[1] = 'S';
	cbw->dCBWSignature[2] = 'B';
	cbw->dCBWSignature[3] = 'C';
	*ret_tag = tag;
	cbw->dCBWTag = tag++;
	cbw->dCBWDataTransferLength = data_length;
	cbw->bmCBWFlags = direction;
	cbw->bCBWLUN = lun;
	cbw->bCBWCBLength = cdb_len;
	memcpy(cbw->CBWCB, cdb, cdb_len);
	
	
	r = usb_bulk_msg(udev,usb_sndbulkpipe(udev,endpoint),(void*)cbw,31, &size,1000);
	
	
	printk("sent %d CDB bytes\n", cdb_len);
	
	return 0;
}

static int test_mass_storage(struct usb_device *udev, uint8_t endpoint_in, uint8_t endpoint_out)
{
int r,size;
uint8_t cdb[16];	// SCSI Command Descriptor Block
uint8_t *buffer=kmalloc(sizeof(uint8_t),GFP_KERNEL);
uint32_t expected_tag;
uint8_t lun=0;
uint32_t i, max_lba, block_size;
long device_size;
unsigned int pipe;

memset(buffer, 0, sizeof(buffer));
memset(cdb, 0, sizeof(cdb));
cdb[0] = 0x25;	// Read Capacity



        r= usb_control_msg(udev,usb_sndctrlpipe(udev,0),BOMS_RESET,BOMS_RESET_REQ_TYPE,0,0,NULL,0,1000);
	if(r<0)
		printk("can not reset ");
	else
		printk("reset has been done");


	

        send_mass_storage_command(udev,endpoint_out,lun,cdb,USB_ENDPOINT_IN,READ_CAPACITY_LENGTH,&expected_tag);
	usb_bulk_msg(udev,usb_rcvbulkpipe(udev,endpoint_in),(void*)buffer,READ_CAPACITY_LENGTH ,&size, 1000);
	printk("received %d bytes\n", size);
        max_lba = be_to_int32(&buffer[0]);
        block_size = be_to_int32(&buffer[4]);
        device_size = ((long)(max_lba+1))*block_size/(1024*1024*1024);
	printk("max_lba: %x",max_lba);
        printk("SIZE OF PENDRIVE is %ld GB\n", device_size);
        get_mass_storage_status(udev, endpoint_in, expected_tag);
       return 0;
}







static int usbdev_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int i;
        struct usb_device *udev = interface_to_usbdev(interface);
         if(udev==NULL)
         {
              printk("no dev struct");
              }
        uint8_t endpoint_in = 0, endpoint_out = 0;	// default IN and OUT endpoints
	unsigned char epAddr, epAttr;
	//struct usb_host_interface *if_desc;
	struct usb_endpoint_descriptor *ep_desc;
        
    if(id->idProduct == Hewlett_Packard_PID)
	{
		printk(KERN_INFO "Hewlett-Packard Plugged in\n");
	}
	else 
	{
		printk(KERN_INFO "Nothing Plugged in\n");
	}


	//if_desc = interface->cur_altsetting;
	printk(KERN_INFO "No. of Altsettings = %d\n",interface->num_altsetting);

	printk(KERN_INFO "No. of Endpoints = %d\n", interface->cur_altsetting->desc.bNumEndpoints);

	for(i=0;i<interface->cur_altsetting->desc.bNumEndpoints;i++)
	{
		ep_desc = &interface->cur_altsetting->endpoint[i].desc;
		epAddr = ep_desc->bEndpointAddress;
		epAttr = ep_desc->bmAttributes;
	        if ((interface->cur_altsetting->desc.bInterfaceClass == 8)&& (interface->cur_altsetting->desc.bInterfaceSubClass == 6) ) 
			{
				// Mass storage devices that can use basic SCSI commands
				//test_mode = USE_SCSI;
				printk(KERN_INFO "USB Device Is a UAS Device ");
			}

		if((epAttr & USB_ENDPOINT_XFERTYPE_MASK)==USB_ENDPOINT_XFER_BULK)
		{
			if(epAddr & USB_ENDPOINT_IN)
                           {

                                   if (!endpoint_in)
                                   endpoint_in = epAddr;
                                   printk(KERN_INFO "EP %d is Bulk IN\n", i);
                             }
			else
                          {

                                   if (!endpoint_out)
                                   endpoint_out = epAddr;
				printk(KERN_INFO "EP %d is Bulk OUT\n", i);
	                    }
		}
               

	}
	//this line causing error
	//p_usbdev_info->class = interface->cur_altsetting->desc.bInterfaceClass;
	
	printk(KERN_INFO "USB DEVICE CLASS : %x", interface->cur_altsetting->desc.bInterfaceClass);
	printk(KERN_INFO "USB DEVICE SUB CLASS : %x", interface->cur_altsetting->desc.bInterfaceSubClass);
	printk(KERN_INFO "USB DEVICE Protocol : %x", interface->cur_altsetting->desc.bInterfaceProtocol);
        printk(KERN_INFO "%s: No. of Altsettings = %d\n",__func__,interface->num_altsetting);
	printk(KERN_INFO "%s: No. of Endpoints = %d\n",__func__, interface->cur_altsetting->desc.bNumEndpoints);
        test_mass_storage(udev,endpoint_in, endpoint_out);
        return 0;
}

/*Operations structure*/
static struct usb_driver usbdev_driver = {
	name: "my_usb_device",  //name of the device
	probe: usbdev_probe, // Whenever Device is plugged in
	disconnect: usbdev_disconnect, // When we remove a device
	id_table: usbdev_table, //  List of devices served by this driver
};



int device_init(void)
{
	usb_register(&usbdev_driver);
	printk(KERN_NOTICE "UAS READ Capacity Driver Inserted\n");
	return 0;
}

void device_exit(void)
{
	usb_deregister(&usbdev_driver);
	printk("Device Driver Unregister");
}

module_init(device_init);
module_exit(device_exit);
MODULE_LICENSE("GPL");

