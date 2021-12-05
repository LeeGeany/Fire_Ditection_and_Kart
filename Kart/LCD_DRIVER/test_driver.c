#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/device.h>

#define DRIVER_NAME "lcd"

struct i2c_client *lcd_i2c;

static const struct i2c_device_id lcd_id[] = {
        { "lcd", 0 },
        { }
};


MODULE_DEVICE_TABLE(i2c, lcd_id);


void lcd_update_device(unsigned long data)
{
        struct i2c_client *client = lcd_i2c;
        i2c_smbus_read_byte_data(client, 0);
}


static struct i2c_driver lcd_driver = {
        .driver.name    = DRIVER_NAME,
        .probe          = lcd_probe,
        .remove         = __devexit_p(lcd_remove),
        .id_table       = lcd_id,
};


static int __devinit lcd_probe(struct i2c_client *client,
                                  const struct i2c_device_id *id)
{
        int status;

        if (!i2c_check_functionality(client->adapter,
                                     I2C_FUNC_SMBUS_WORD_DATA)) {
                dev_err(&client->dev, "adapter doesn't support SMBus word "
                        "transactions\n");
                return -ENODEV;
        }


        lcd_i2c = client;
        dev_info(&client->dev, "initialized\n");

        return 0;
}


static int __init lcd_init(void)
{
        return i2c_add_driver(&lcd_driver);
}

module_init(lcd_init);

static void __exit lcd_exit(void)
{
        i2c_del_driver(&lcd_driver);
}
module_exit(lcd_exit);
