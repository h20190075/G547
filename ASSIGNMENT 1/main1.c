#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/moduleparam.h>
#include <linux/version.h> 
#include <linux/types.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>   
#include <linux/cdev.h> 



#include<linux/slab.h>                
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>

#include <linux/random.h>
 

#define WR_VALUE _IOW('a','a',int32_t*)
#define WR_VALUE2 _IOW('a','b',int32_t*)
 
int32_t value ,j=0,value_a;


static int out;
char buff[16];
char buff1[16];
char buff2[16]="invalid channel";


uint16_t random,temp;

static dev_t first;
static struct cdev c_dev;
static struct class *cls;
 

static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
 

 
static int my_open(struct inode *i,struct file *f)
{

printk(KERN_INFO "mychar:open()\n");
return 0;

}
static int my_close(struct inode *i,struct file *f)
{

printk(KERN_INFO "mychar:close()\n");
return 0;

}

static ssize_t my_read(struct file *f,char __user *buf,size_t len,loff_t *off)
{
if(value>=0 & value<=7)
{

int i=0,k=0;

get_random_bytes(&random,sizeof(random));

random=(random%1024); 
temp=random;
if(random<=512)
{ 

random =random+512; 

}

while(random!=0)
{

buff[i]=(random%2)+'0';
random=random/2;
i=i+1;
}

buff[i]='\0';


if(value_a==0)
{
k=10;
while(k<=15)
{
buff1[k]='a';
k=k+1;
}
k=0;
while(k<=9)
{
buff1[k]=buff[k];
k=k+1;
}

}


int l=0;
if(value_a==1)
{
while(l<=5)
{
buff1[l]='a';
l=l+1;
}
l=6;
while(l<=15)
{
buff1[l]=buff[l-6];
l=l+1;
}

}


buff1[16]='\0';

printk("buff1  %s",buff1);
printk("temp  %d",temp);

printk(KERN_INFO "mychar: read()\n");
if(copy_to_user(buf,buff1,sizeof(buff1)));

}
else
{
if(copy_to_user(buf,buff2,sizeof(buff2)));
}

return 0;

}
 
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
       
 switch(cmd) {
                case WR_VALUE:
                        value=arg;
                        printk(KERN_INFO "Value = %ld \n", arg);

                  case WR_VALUE2:
                        value_a=arg;
                              
                        printk(KERN_INFO "Value_a = %d \n", value_a);
                      
                        
           
        }
        return 0;
}
 
static struct file_operations fops=
                                      {

                                         .owner=THIS_MODULE,
                                         .open=my_open,
                                         
                                         .release=my_close,
                                         .read=my_read,
                                          .unlocked_ioctl = etx_ioctl
                                       

                                          };

 
static int __init mychar_init(void)
{
     printk(KERN_INFO "namaste: mychar drive registered");

      //step 1
    if(alloc_chrdev_region(&first,0,1,"bits-pilani")<0)
    {
       return -1;
      }
      //step 2
   if((cls=class_create(THIS_MODULE, "chardrv"))==NULL)
    {
       unregister_chrdev_region(first,1);
       return -1;
     }
    if(device_create(cls,NULL,first,NULL,"adc8")==NULL)
   {
    class_destroy(cls);
    unregister_chrdev_region(first,1);
   return -1;
   }
   //step3
   cdev_init(&c_dev,&fops);
   if(cdev_add(&c_dev,first,1)==-1)
   {
     device_destroy(cls,first);
    class_destroy(cls);
    unregister_chrdev_region(first,1);
   return -1;
     } 
return 0;
}


static void __exit mychar_exit(void)
{
    cdev_del(&c_dev);
    device_destroy(cls,first);
    class_destroy(cls);
    unregister_chrdev_region(first,1);
    printk(KERN_INFO "mychar driver unregistered\n\n");

}

module_init(mychar_init);
module_exit(mychar_exit);

 
MODULE_LICENSE("GPL");


