/*** Network Device Drivers mynetdev01.c
#  *
#  * 14 Sept 2020 Nisa Mercan <nisamercan11@gmail.com>
#  * 15 Sept 2020 -update-
#  *
#  ***/

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>

static struct net_device* dev;         /* network device */
static struct net_device_stats* stats; /*   packet stats */


/* Loopback
 * To receive the packet and send the packet back to kernel using netif_rx.
 */
static void mynetdev_rx(struct sk_buff* skb, struct net_device* dev) {
    printk(KERN_INFO "@mynetdev_rx: Receiving the packet\n");
    ++stats->rx_packets;
    printk("@mynetdev_get_stats: Received packets: %ld\n", stats->rx_packets);
    netif_rx(skb);         /* Receives the packet from a device driver
                              and queues it for the upper levels to process */
}


/* Transmit
 * Initiates the transmission of a packet
 */
static int mynetdev_hard_start_xmit(struct sk_buff* skb, struct net_device* dev) {
    int i;
    printk(KERN_INFO "@mynetdevhard_start_xmit: Sending the packet. \n");
    netif_trans_update(dev); /* to save the timestamp */
      //  for (i = 0; i < skb->len; ++i) {
      //          if ((i & 0xf) == 0) {
      //              printk("\t");
      //          }
      //      printk("@packet: %02x", skb->data[i]);
      //  }
    printk("\n");
    ++stats->tx_packets;
    printk("@mynetdev_get_stats: Sent packets: %ld\n", stats->tx_packets);
    mynetdev_rx(skb, dev); /*send back to the kernel using loopback to receive packets */                       //dev_kfree_skb(skb); this is only for sending packets.
    return 0;
}


/* Do Ioctl
 * Perform interface-specific ioctl commands
 */
static int mynetdev_do_ioctl(struct net_device* dev, struct ifreq* ifr, int cmd) {
    printk(KERN_INFO "@mynetdev_do_ioctl: Opened. \n");
    return 0;
}


/* Get stats
 * To get statistics for the interface when ifconfig or netstat -i run
 */
static struct net_device_stats* mynetdev_get_stats(struct net_device* dev) {
    printk(KERN_INFO "@mynetdev_get_stats: Opened. \n");
    return stats;
}


/* Set Config
 * To change the interface configuration
 */
static int mynetdev_set_config(struct net_device* dev, struct ifmap* map) {
    printk("@mynetdev_set_config: Opened. \n");
    if (dev->flags & IFF_UP) {  /* IFF_UP: Read-only for the driver.
                                 * Kernel turns it on when the interface is ready to transfer packets */
        return -EBUSY;
    }
    return 0;
}


/* Change MTU
 * To make change in the MTU (maximum transfer unit)
 */
static int mynetdev_change_mtu(struct net_device* dev, int new_mtu) {
    printk("@mynetdev_change_mtu: Opened. \n");
    return 0;
}


/* Time Out
 * When a packet transmission fails to complete within a reasonable period,
 * on the assumption that an interrupt has been missed or the interface has locked up
 * handle the problem and resume packet transmission
 */
static void mynetdev_tx_timeout(struct net_device* dev) {
    printk("@mynetdev_tx_timeout: Opened\n");
    printk("@mynetdev_tx_timeout: Transmit timeout at %ld\n", jiffies);
    netif_wake_queue(dev); /* restarts the transmission queue */
}


/* Open
 * Opens the interface
 */
static int mynetdev_open(struct net_device* dev) {
    printk(KERN_INFO "@mynetdev_open: Opened the device %s\n", dev->name);
    netif_start_queue(dev);     /* Start up the transmission queue.
                                 * Allow upper layers to call the device hard_start_xmit routine.
                                 * Until this call, the device won't be available. */
    return 0;
}


/* Stop
 * Stops the interface
 */
static int mynetdev_stop(struct net_device* dev) {
    printk(KERN_INFO "@mynetdev_stop: Stopped the device %s\n", dev->name);
    netif_stop_queue(dev);      /* Shutdown the transmission queue.
                                 * Stop upper layers calling the device hard_start_xmit routine. */
    return 0;
}


static const struct net_device_ops mynetdev_ops = {
  .ndo_open = mynetdev_open,
  .ndo_stop = mynetdev_stop,
  .ndo_start_xmit = mynetdev_hard_start_xmit,
  .ndo_get_stats = mynetdev_get_stats,
  .ndo_do_ioctl = mynetdev_do_ioctl,
  .ndo_set_config = mynetdev_set_config,
  .ndo_change_mtu = mynetdev_change_mtu,
  .ndo_tx_timeout = mynetdev_tx_timeout,
};


/* Setting up the device
 * fields, addresses etc.
 */
static void mynetdev_setup(struct net_device* dev) {
    int j;
    printk("@mynetdev_setup: Opened. Setting up.\n");

    for (j = 0; j < ETH_ALEN; ++j) { /* Fill in the MAC address with '00:01:02:03:04:05' */
        dev->dev_addr[j] = (char)j;
    }

    ether_setup(dev); /* Fill in the fields of the device with ethernet-generic values. */
    dev->netdev_ops = &mynetdev_ops;
    dev->flags |= IFF_PROMISC; /* set to activate promiscuous operation device */
    //dev->watchdog_timeo = jiffies;
    stats = &dev->stats;      /* device stats to stats structure */

}


/* Init
 */
static int __init mynetdev_init(void) {
    printk("\n\n");
    printk(KERN_INFO "@mynetdev_init: Loading the network module");

    dev = alloc_netdev(0, "mynetdev%d", NET_NAME_UNKNOWN, mynetdev_setup); /*allocate a struct net_device*/
    /* if (dev == NULL) {
        return -ENOMEM;
    } */
    if (register_netdev(dev)) { /*error check*/
        printk(KERN_INFO "@mynetdev_init: Failed to register\n");
        free_netdev(dev);
        return -1;
    }
    printk(KERN_INFO "@mynetdev_init: Module loaded. Success.\n");
    return 0;
}

/* Exit
 */
static void __exit mynetdev_exit(void) {
    printk(KERN_INFO "@mynetdev_exit: Unloading the network module\n");
    unregister_netdev(dev);
    free_netdev(dev);
}

module_init(mynetdev_init);
module_exit(mynetdev_exit);

MODULE_LICENSE("GPL"); /*General Public License*/
MODULE_AUTHOR("Nisa Mercan <nisamercan11@gmail.com>");
MODULE_DESCRIPTION("Network Device Driver");
