#include "OUR_IMU.h"
#include "math.h"
#include "string.h"
#include "esp_timer.h"

// Variables to hold data on the gyro and accleration from IMU
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX = 0.0;
float AccErrorY = 0.0;
float GyroErrorX = 0.0;
float GyroErrorY = 0.0;
float GyroErrorZ = 0.0;
int c = 0;

float local_elaspedTime, previousTime;
float currentTime = 0.0;

// Configuration setting to start I2C driver
// creates driver/board to be a Master and set SDA and SCL lines to be pull up and pins 21 and 22
i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_MASTER_FREQ_HZ,
};

// Officially starts the boards I2C driver
esp_err_t our_i2c_master_init(void){
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

// Helper function that first writes to the mpu6050 and request data from certain register
// it will then read the number of bytes specificed from that address and store it into data
esp_err_t mpu6050_register_read(uint8_t reg_addr, uint8_t *data, size_t len){
    return i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_SENSOR_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

// Helper function to just write a single byte a register inside of the mpu6050
esp_err_t mpu6050_register_write_byte(uint8_t reg_addr, uint8_t data){
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;  
}

void calculate_IMU_error(void){
    uint8_t data[2];

    while(c < 400){
        
        mpu6050_register_read(MPU6050_GYRO_XOUT, data, 2);
        GyroX = (int16_t)((data[0] << 8) | (data[1]));
        mpu6050_register_read(MPU6050_GYRO_YOUT, data, 2);
        GyroY = (int16_t)((data[0] << 8) | data[1]);
        mpu6050_register_read(MPU6050_GYRO_ZOUT, data, 2);
        GyroZ = (int16_t)((data[0] << 8) | data[1]);

        GyroErrorX = GyroErrorX + (GyroX / 131.0);
        GyroErrorY = GyroErrorY + (GyroY / 131.0);
        GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
        c++;
    }
    GyroErrorX = GyroErrorX / 400;
    GyroErrorY = GyroErrorY / 400;
    GyroErrorZ = GyroErrorZ / 400;

    c = 0;
    while(c < 400){
        mpu6050_register_read(MPU6050_ACCEL_XOUT, data, 2);
        AccX = (int16_t)((data[0] << 8) | data[1]) / 16384.0;
        mpu6050_register_read(MPU6050_ACCEL_YOUT, data, 2);
        AccY = (int16_t)((data[0] << 8) | data[1]) / 16384.0;
        mpu6050_register_read(MPU6050_ACCEL_ZOUT, data, 2);
        AccZ = (int16_t)((data[0] << 8) | data[1]) / 16384.0;

        AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / M_PI));    
        AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / M_PI));

        c++;
    }
    AccErrorX = AccErrorX / 400;
    AccErrorY = AccErrorY / 400;

    ESP_LOGI(TAG_IMU, "AccErrorX: %f", AccErrorX);
    ESP_LOGI(TAG_IMU, "AccErrorY: %f", AccErrorY);
    ESP_LOGI(TAG_IMU, "GyroErrorX: %f", GyroErrorX);
    ESP_LOGI(TAG_IMU, "GyroErrorY: %f", GyroErrorY);
    ESP_LOGI(TAG_IMU, "GyroErrorZ: %f", GyroErrorZ);
}

void calculate_Angles(int64_t elapsed_time, float *ret_roll, float *ret_pitch, float *ret_yaw){
    uint8_t data[2];
    
    mpu6050_register_read(MPU6050_ACCEL_XOUT, data, 2);
    AccX = (int16_t)((data[0] << 8) | data[1]) / 16384.0;
    mpu6050_register_read(MPU6050_ACCEL_YOUT, data, 2);
    AccY = (int16_t)((data[0] << 8) | data[1]) / 16384.0;
    mpu6050_register_read(MPU6050_ACCEL_ZOUT, data, 2);
    AccZ = (int16_t)((data[0] << 8) | data[1]) / 16384.0;

    accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / M_PI) - 0.58; // AccErrorX ~(0.58) See the calculate_IMU_error()custom function for more details
    accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / M_PI) + 1.58; // AccErrorY ~(-1.58)

    // Elapsed Time here
    previousTime = currentTime;
    currentTime = (float)(esp_timer_get_time());
    local_elaspedTime = (currentTime - previousTime) / 1000000;

    mpu6050_register_read(MPU6050_GYRO_XOUT, data, 2);
    GyroX = (int16_t)((data[0] << 8) | data[1]);
    mpu6050_register_read(MPU6050_GYRO_YOUT, data, 2);
    GyroY = (int16_t)((data[0] << 8) | data[1]);
    mpu6050_register_read(MPU6050_GYRO_ZOUT, data, 2);
    GyroZ = (int16_t)((data[0] << 8) | data[1]);

    GyroX = (GyroX / 131.0) + 1.56; // GyroErrorX ~(-0.56)
    GyroY = (GyroY / 131.0) + 2.5; // GyroErrorY ~(2)
    GyroZ = (GyroZ / 131.0) + 0.8; // GyroErrorZ ~ (-0.8)

    gyroAngleX = gyroAngleX + (GyroX * local_elaspedTime);//elapsed_time;
    gyroAngleY = gyroAngleY + (GyroY * local_elaspedTime);//elapsed_time;

    yaw =  yaw + GyroZ * local_elaspedTime;//elapsed_time;
    roll = (0.96 * gyroAngleX) + (0.04 * accAngleX);
    pitch = (0.96 * gyroAngleY) + (0.04 * accAngleY);

    *ret_yaw = yaw;
    *ret_roll = roll;
    *ret_pitch = pitch;
}