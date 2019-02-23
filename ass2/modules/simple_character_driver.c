#include<linux/init.h>
#include<linux/module.h>
#include<linux/slab.h>
#include<linux/fs.h>
#include<linux/uaccess.h>

#define buffSize 1024

char * device_buffer;

int numOpens = 0;
int numCloses = 0;
int buffPos = 0;

loff_t my_seek(struct file *f, loff_t offset, int whence){
	/*
Seek to whatever position is specified by the offset in combination with the whence
	*/
	loff_t newPos;
	switch(whence){

		case 0: //SEEK_SET set position to offset
			newPos = offset;
			break;

		case 1: //SEEK_CUR set position to offset plus current position
			newPos = buffPos+offset;
			break;

	 	case 2: //SEEK_END set position to end of buffer - offset
			newPos = buffSize - offset;
			break;

		default: //error
			return -1;
	}
	if(newPos<0) return -1;
	buffPos = newPos;
	f->f_pos = newPos;
	//printk(KERN_ALERT "Seeked? to position %lld \n",f->f_pos);
	return newPos;
}
ssize_t my_read (struct file *f, char __user *user_buffer, size_t length, loff_t *offset)
{
	//printk(KERN_ALERT "Value of the offset in read function is : %lld\n", *offset);
	//printk(KERN_ALERT "Value of the file position in read function is : %lld\n", f->f_pos);

	/*
Want to read from the current position saved to buffPos length number of bytes
	*/

	//check to make sure offset is currently at a place which makes sense
	if (*offset > buffSize)
	{
		 printk(KERN_ALERT "offset is greater than buffer size silly!\n");
		 return 0;
	}
	//can't read more than the size of the buffer
	if (*offset + length + buffPos> buffSize)
	{
		 length = buffSize - *offset;
	}
	if (copy_to_user(user_buffer, device_buffer + buffPos + *offset, length) != 0)
 	{
		 return -EFAULT;
 	}
	printk(KERN_ALERT "Read from the driver. Number of bytes read : %ld\n", length);
	*offset += length;
	buffPos +=length;
	//printk(KERN_ALERT "Now, after reading, the value of the offset is %ld\n", *offset);
 return length;
}



ssize_t my_write (struct file *f, const char __user *user_buffer, size_t length, loff_t *offset)
{
	/*
write from current position length number of bytes if possible
	*/
	int writeBytes;
	int remaining = buffSize - *offset - buffPos;
	int err;
	//printk(KERN_ALERT "Value of the offset in write function is : %lld\n", *offset);
	//printk(KERN_ALERT "Value of the file position in write function is : %lld\n", f->f_pos);
	// if we have enough room, write the whole message
	if(remaining > length){
		writeBytes = length;
	}
	else{ //if not, write only the portion of message we have room for
		writeBytes = remaining;
	}
	//copy from user returns how many bytes we were unable to copy if there wasnt enough room
	err = copy_from_user(device_buffer + *offset + buffPos, user_buffer, writeBytes);

	//if we've reached the end of the buffer, notify kernel
	if(err >= 1){
		printk(KERN_ALERT "No Mo Space Left.\n");
	}
	else{
		//We were able to at least write some of the message to the file, so change current position
		*offset += writeBytes;
		buffPos += writeBytes;

		printk(KERN_ALERT "Wrote to the driver. Number of bytes written : %d\n", writeBytes);
	}
	return writeBytes;
}


int my_open (struct inode *n, struct file *f)
{
	numOpens++;
	printk(KERN_ALERT "Opened the driver. Number of times opened: %d\n", numOpens);
	return 0;
}


int my_close (struct inode *n, struct file *f)
{
	numCloses++;
	printk(KERN_ALERT "Closed the driver. Number of times closed: %d\n", numCloses);
	return 0;
}

struct file_operations fopstruct = {
	.read    = my_read,
	.write   = my_write,
	.open    = my_open,
	.llseek  = my_seek,
	.release = my_close
};

static int my_init(void)
{
	printk(KERN_ALERT "Currently in the INIT function\n");
	//initialize buffer and register device
	device_buffer = kmalloc(1024, GFP_KERNEL);
	register_chrdev( 300, "simple_c_driver", &fopstruct);
	printk(KERN_ALERT "Successfully registered device with major number 300\n");
	return 0;
}

static void my_exit(void)
{
	printk(KERN_ALERT "Currently in the EXIT function\n");
	//Free the buffer memory and unregister driver
	kfree(device_buffer);
	unregister_chrdev( 300, "simple_c_driver");

}

module_init(my_init);
module_exit(my_exit);
