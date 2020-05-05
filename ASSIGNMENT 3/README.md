

Shivam kumar tripathi     2019H1400533G

Riya Bisht                2019H1400075G

..>>>>>>>>>>>>>>>>>>> USB BLOCK DRIVER CODE FOR READING AND WRITING FILES IN USB USING SCSI COMMANDS <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 

step 1: Go to the directory where yor code is there and give the command  $ make all

step 2: remove kernel drivers using commands   $ sudo rmmod uas 
                                               $ sudo rmmod usb_storage 

step 3: insert the module using  $ sudo insmod main.ko

step 4: check whether module is loaded using command  $ lsmod 

step 5: connect pendrive ,then again follow step 2 (as they may get installed again)

step 6: check wether your usb is using your driver or not $ sudo fdisk -l

step 7: make folder inside media directory  $ sudo mkdir /media/shivamusb 

step 8: use mount command for accessing pendrive  $ sudo mount -t vfat /dev/USBDRIVER /media/shivamusb

step 9: go inside root  $ sudo -i

step 10: go inside pusb folder of media directory  $ cd /media/shivamusb/

step 11: command to see content of pendrive  $ ls

step 12: command to create new text file and writing into it  $ echo everyone>test12.txt

step 13: command to read content of file  $ cat test12.txt

step 14: command to come outside of root  $ logout

step 15: remove pendrive and again connect ,check its content you can clearly see new file test12.txt in it along with already existing files


