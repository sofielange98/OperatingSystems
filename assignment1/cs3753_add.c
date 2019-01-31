#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>
asmlinkage long sys_cs3753_add(int n1, int n2, int * addy)
{
 /* first log the two variables we recieved*/
 int sum = n1+n2;
 printk(KERN_ALERT "First value to add : %d\n",n1);
 printk(KERN_ALERT "Second value to add : %d\n",n2);
 copy_to_user(addy,&sum,4);
 printk(KERN_ALERT "Added together = %d\n",sum);
 return 0;
} 
