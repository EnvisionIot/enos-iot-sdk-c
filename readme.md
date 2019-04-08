# Using EnOS IoT SDK for C
This article instructs how to prepare your development environment to use the EnOS IoT SDK for C.

- Build and Install
    - [Install CMake](#installcmake)
    - [Compile](#compile)
- [Feature List](#features)
- Sample Code
    - [RSA encryption and decryption](#rsa-encyrpt-decrypt)
    - [RSA encryption and decryption with TPM2 chips](#rsa-encyrpt-decrypt-tpm2)
    - [Generate certificate request](#csr)
    - [Hash](#hash)
    - [Connect to EnOS Cloud](#connect)
    - [Connect to EnOS Cloud through SSL/TLS](#connect-ssl)
    - [Add Topology of Sub-devices](#add-topology)
    - [Proxy sub-devices to log into EnOS Cloud](#connect-subdevice)
    - [Report Measure Points](#report-measure-points)
    - [Invoke Device Services](#invoke-service)
    - [Get Thing Model](#get-model)
    - [Apply Certificate](#apply-cert)
- [Project File Structure](#project)
- [Start to Use](#how-to)

## Build and Install

<a name="installcmake"></a>
### Install CMake
To use the EnOS IoT SDK for C, you will need to install CMake, download CMake from https://cmake.org/.

<a name="compile"></a>
### Compile

To Compile in **Debug** mode:
```
    cmake -D PLATFORM_DEFINE=linux_x86_normal_64_local -D DEBUG_MODE=ON .
    make //or use "make VERBOSE=1" to get more make informations
```

To compile in **Release** mode:
```
    cmake -D PLATFORM_DEFINE=linux_x86_normal_64_local -D DEBUG_MODE=OFF .
    make //or use "make VERBOSE=1" to get more make informations
```

Where, the commands and parameters are explained as follows:

1. Compile result `dir: dst`

2. Use `PLATFORM_DEFINE=<platform type>` to choose build and target platform. The format of platform type is `os_arch_desc_bits_local-or-cross`, for example: 
   - linux_x86_normal_64_local
   - linux_armv7l_moxa_32_local
   - linux_x86_centos6_32_local
   - win_x86_mingw_32_local
            
3. Use `-G "xxx"` to choose different CMake result, for example:
   ```
   -G "MSYS Makefiles"      --msys on windows
   -G "MinGW Makefiles"     --mingw on windows
   -G "Visual Studio 14"    --vs2015 project on windows(not supported now)
   ```
            
4. Use `ENABLE_TPM=<ON/OFF>` to enable or disable tpm2 support, The tpm2 API works only in linux server with tpm2 chips or simulator.
    
5. Use `./clean_all_cmake_tmp.sh` to clean all CMake tmp files and directories (CMake will generate temporary files and directories, aka the CMakeFiles such as `cmake_install.cmake`, `CMakeCache.txt`, `Makefile`.
        
        
6. `support/linux_x86_normal_64_local` is precompiled in CentOS_7.4_x64 and `support/win_x86_mingw_32_local` is precompiled in win7_msys2-i686 with `i686-4.8.5-release-posix-sjlj-rt_v4-rev0(mingw-w64)`


<a name="features"></a>
## Key Features

The EnOS IoT SDK for C supports the following functions:
- enos_encrypt_api: APIs for X.509 certificate-based authentication
    - Generate RSA public and private keys
    - Generate RSA public and private keys with TPM2 chips or simulator
    - Generate certificate request
    - Hash
    - Certificate validation
    - RSA encryption and decryption
    - RSA encryption and decryption with TPM2 chips or simulator

- enos_mqtt_api: APIs for data transmission between devices and cloud through MQTT protocol
    - Registration of devices
    - Add, delete and get topology of sub-devices
    - Online and offline of sub-devices
    - Report, query and delete device tags
    - Report, query and delete device attributes
    - Get thing model
    - Invoke device services
    - Get and set value of device measure points
    - Upload device messages
    - Enable, disable and delete devices or sub-devices

- enos_restful_api: APIs for EnOS Cloud services
    - Apply, revoke and list certificate
    - Create, delete, update and get product
    - Download file
    - Get thing model

<a name="sample-code"></a>
## Sample Code

The following sample codes instruct how to use the EnOS IoT SDK for C.
### enos_encrypt_api

<a name="ras-encyrpt-decrypt"></a>
#### RSA encryption and decryption
The sample code encrypts data using public key and decrypts data using private key:

```
int rsa_public_encrypt_private_decrypt_test()
{
    int ret = 0;
    
    char pub_key_path[1024];
    memset(pub_key_path, 0, sizeof(pub_key_path));
    snprintf(pub_key_path, sizeof(pub_key_path), "%s", "./enos_encrypt_test_tmp/pub_key.pem");
    
    char pri_key_path[1024];
    memset(pri_key_path, 0, sizeof(pri_key_path));
    snprintf(pri_key_path, sizeof(pri_key_path), "%s", "./enos_encrypt_test_tmp/pri_key.pem");
    
    char cipher_fun[1024];
    memset(cipher_fun, 0, sizeof(cipher_fun));
    snprintf(cipher_fun, sizeof(cipher_fun), "%s", "aes256");
    
    char key_passwd[1024];
    memset(key_passwd, 0, sizeof(key_passwd));
    snprintf(key_passwd, sizeof(key_passwd), "%s", "123456");
    
    int key_size = 2048;
    EVP_PKEY *pkey = NULL;
    
    ret = enos_generate_rsa_key(pub_key_path, pri_key_path, cipher_fun, key_passwd, key_size, &pkey);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_generate_rsa_key error\n");
        return -1;
    }
    
    EVP_PKEY *pri_key = NULL;
    ret = enos_read_rsa_pri_key_from_file(pri_key_path, key_passwd, &pri_key);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_read_rsa_pri_key_from_file error\n");
        EVP_PKEY_free(pkey);
        return -1;
    }
    
    EVP_PKEY *pub_key = NULL;
    ret = enos_read_rsa_pub_key_from_file(pub_key_path, &pub_key);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_read_rsa_pub_key_from_file error\n");
        EVP_PKEY_free(pkey);
        EVP_PKEY_free(pri_key);
        return -1;
    }
    
    char origin_text[1024];
    int ii = 0;
    for(ii = 0; ii < (int)(sizeof(origin_text)); ii++)
    {
        origin_text[ii] = ii & 0xff;
    }
    
    char enc_text[2048];
    memset(enc_text, 0, sizeof(enc_text));
    
    char dec_text[2048];
    memset(dec_text, 0, sizeof(dec_text));
    
    int enc_len = 0;
    int dec_len = 0;
    
    for(ii = 0; ii < 3; ii++)
    {
        memset(enc_text, 0, sizeof(enc_text));
        enc_len = 0;
        ret = enos_rsa_public_encrypt_advance(pkey, origin_text, (int)(sizeof(origin_text)), enc_text, (int)(sizeof(enc_text)), &enc_len);
        if(ret < 0)
        {
        	printf("enos_rsa_public_encrypt_advance error\n");
        	EVP_PKEY_free(pkey);
            EVP_PKEY_free(pri_key);
            EVP_PKEY_free(pub_key);
        	return -1;
        }
        memset(dec_text, 0, sizeof(dec_text));
        dec_len = 0;
        ret = enos_rsa_private_decrypt_advance(pkey, enc_text, enc_len, dec_text, (int)(sizeof(dec_text)), &dec_len);
        if(ret < 0)
        {
        	printf("enos_rsa_private_decrypt_advance error\n");
        	EVP_PKEY_free(pkey);
            EVP_PKEY_free(pri_key);
            EVP_PKEY_free(pub_key);
        	return -1;
        }
        
        if(memcmp(origin_text, dec_text, dec_len) == 0)
        {
            printf("ii=%d, pkey test success\n", ii);
        }
        else
        {
            printf("ii=%d, pkey test fail\n", ii);
        }
    }
    
    for(ii = 0; ii < 3; ii++)
    {
        memset(enc_text, 0, sizeof(enc_text));
        enc_len = 0;
        ret = enos_rsa_public_encrypt_advance(pub_key, origin_text, (int)(sizeof(origin_text)), enc_text, (int)(sizeof(enc_text)), &enc_len);
        if(ret < 0)
        {
        	printf("enos_rsa_public_encrypt_advance error\n");
        	EVP_PKEY_free(pkey);
            EVP_PKEY_free(pri_key);
            EVP_PKEY_free(pub_key);
        	return -1;
        }
        memset(dec_text, 0, sizeof(dec_text));
        dec_len = 0;
        ret = enos_rsa_private_decrypt_advance(pri_key, enc_text, enc_len, dec_text, (int)(sizeof(dec_text)), &dec_len);
        if(ret < 0)
        {
        	printf("enos_rsa_private_decrypt_advance error\n");
        	EVP_PKEY_free(pkey);
            EVP_PKEY_free(pri_key);
            EVP_PKEY_free(pub_key);
        	return -1;
        }
        
        if(memcmp(origin_text, dec_text, dec_len) == 0)
        {
            printf("ii=%d, pub_key pri_key test success\n", ii);
        }
        else
        {
            printf("ii=%d, pub_key pri_key test fail\n", ii);
        }
    }
    
    EVP_PKEY_free(pkey);
    EVP_PKEY_free(pri_key);
    EVP_PKEY_free(pub_key);
    return 0;
}
```
<a name="rsa-encyrpt-decrypt-tpm2"></a>
#### RSA encryption and decryption with TPM2 chips

```
int rsa_public_encrypt_private_decrypt_tpm2_test()
{
    int ret = 0;
    
    char tpm2_key_path[1024];
    memset(tpm2_key_path, 0, sizeof(tpm2_key_path));
    snprintf(tpm2_key_path, sizeof(tpm2_key_path), "%s", "./enos_encrypt_test_tmp/rsa_key.tss");
    
    char *owner_hierarchy_passwd = NULL;
    
    char key_passwd[1024];
    memset(key_passwd, 0, sizeof(key_passwd));
    snprintf(key_passwd, sizeof(key_passwd), "%s", "123456");
    
    int key_size = 2048;
    EVP_PKEY *pkey = NULL;
    
    ret = enos_generate_rsa_key_tpm2(tpm2_key_path, owner_hierarchy_passwd, key_passwd, key_size, &pkey);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_generate_rsa_key_tpm2 error\n");
        return -1;
    }
    
    char origin_text[1024];
    int ii = 0;
    for(ii = 0; ii < (int)(sizeof(origin_text)); ii++)
    {
        origin_text[ii] = ii & 0xff;
    }
    
    char enc_text[2048];
    memset(enc_text, 0, sizeof(enc_text));
    
    char dec_text[2048];
    memset(dec_text, 0, sizeof(dec_text));
    
    int enc_len = 0;
    int dec_len = 0;
    
    for(ii = 0; ii < 3; ii++)
    {
        memset(enc_text, 0, sizeof(enc_text));
        enc_len = 0;
        ret = enos_rsa_public_encrypt_advance(pkey, origin_text, (int)(sizeof(origin_text)), enc_text, (int)(sizeof(enc_text)), &enc_len);
        if(ret < 0)
        {
        	printf("enos_rsa_public_encrypt_advance error\n");
        	EVP_PKEY_free(pkey);
        	return -1;
        }
        memset(dec_text, 0, sizeof(dec_text));
        dec_len = 0;
        ret = enos_rsa_private_decrypt_advance(pkey, enc_text, enc_len, dec_text, (int)(sizeof(dec_text)), &dec_len);
        if(ret < 0)
        {
        	printf("enos_rsa_private_decrypt_advance error\n");
        	EVP_PKEY_free(pkey);
        	return -1;
        }
        
        if(memcmp(origin_text, dec_text, dec_len) == 0)
        {
            printf("ii=%d, pkey test success\n", ii);
        }
        else
        {
            printf("ii=%d, pkey test fail\n", ii);
        }
    }
    EVP_PKEY_free(pkey);
    
    pkey = NULL;
    ret = enos_read_rsa_key_from_file_tpm2(tpm2_key_path, owner_hierarchy_passwd, key_passwd, &pkey);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_read_rsa_key_from_file_tpm2 error\n");
        return -1;
    }
    
    for(ii = 0; ii < 3; ii++)
    {
        memset(enc_text, 0, sizeof(enc_text));
        enc_len = 0;
        ret = enos_rsa_public_encrypt_advance(pkey, origin_text, (int)(sizeof(origin_text)), enc_text, (int)(sizeof(enc_text)), &enc_len);
        if(ret < 0)
        {
        	printf("enos_rsa_public_encrypt_advance error\n");
        	EVP_PKEY_free(pkey);
        	return -1;
        }
        memset(dec_text, 0, sizeof(dec_text));
        dec_len = 0;
        ret = enos_rsa_private_decrypt_advance(pkey, enc_text, enc_len, dec_text, (int)(sizeof(dec_text)), &dec_len);
        if(ret < 0)
        {
        	printf("enos_rsa_private_decrypt_advance error\n");
        	EVP_PKEY_free(pkey);
        	return -1;
        }
        
        if(memcmp(origin_text, dec_text, dec_len) == 0)
        {
            printf("ii=%d, pkey test2 success\n", ii);
        }
        else
        {
            printf("ii=%d, pkey test2 fail\n", ii);
        }
    }
    EVP_PKEY_free(pkey);
    
    return 0;
}
```

<a name="csr"></a>
#### Generate certificate request

The sample code generates a certificate request:

```
int enos_generate_cert_req_test()
{
    int ret = 0;
    
    char pub_key_path[1024];
    memset(pub_key_path, 0, sizeof(pub_key_path));
    snprintf(pub_key_path, sizeof(pub_key_path), "%s", "./enos_encrypt_test_tmp/csr_pub_key.pem");
    
    char pri_key_path[1024];
    memset(pri_key_path, 0, sizeof(pri_key_path));
    snprintf(pri_key_path, sizeof(pri_key_path), "%s", "./enos_encrypt_test_tmp/csr_pri_key.pem");
    
    char req_path[1024];
    memset(req_path, 0, sizeof(req_path));
    snprintf(req_path, sizeof(req_path), "%s", "./enos_encrypt_test_tmp/req.csr");
    
    char cipher_fun[1024];
    memset(cipher_fun, 0, sizeof(cipher_fun));
    snprintf(cipher_fun, sizeof(cipher_fun), "%s", "aes256");
    
    char key_passwd[1024];
    memset(key_passwd, 0, sizeof(key_passwd));
    snprintf(key_passwd, sizeof(key_passwd), "%s", "123456");
    
    int key_size = 2048;
    EVP_PKEY *pkey = NULL;
    
    ret = enos_generate_rsa_key(pub_key_path, pri_key_path, cipher_fun, key_passwd, key_size, &pkey);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_generate_rsa_key error\n");
        return -1;
    }
    
    struct enos_cert_info input_info;
    memset(&input_info, 0, sizeof(struct enos_cert_info));
    input_info.cert_version = ENOS_CERT_VERSION_3;
    input_info.sig_alg = ENOS_DEFAULT_SIG_ALG;
    snprintf(input_info.C_country, sizeof(input_info.C_country), "%s", ENOS_DEFAULT_REQ_C);
    snprintf(input_info.ST_province, sizeof(input_info.ST_province), "%s", ENOS_DEFAULT_REQ_ST);
    snprintf(input_info.L_city, sizeof(input_info.L_city), "%s", ENOS_DEFAULT_REQ_L);
    snprintf(input_info.O_organization, sizeof(input_info.O_organization), "%s", ENOS_DEFAULT_REQ_O);
    snprintf(input_info.OU_organizationunit, sizeof(input_info.OU_organizationunit), "%s", ENOS_DEFAULT_REQ_OU);
    snprintf(input_info.CN_commonname, sizeof(input_info.CN_commonname), "%s", "device SN");
    
    ret = enos_generate_cert_req(req_path, &input_info, pkey);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_ENCRYPT_TEST_LOG_ERROR, "[ENOS_ENCRYPT_TEST]enos_generate_cert_req error\n");
        EVP_PKEY_free(pkey);
        return -1;
    }
    
    return 0;
}
```

<a name="hash"></a>
#### Hash

The sample code hashes an input value.

```
int enos_hash_test()
{
    char *input = "abc123";
    int input_len = strlen(input);
    char *hash_fun = "sha1";
    
    char output[256];
    memset(output, 0, sizeof(output));
    int output_max = sizeof(output);
    
    int output_len = 0;
    int ret = 0;
    
    ret = enos_hash(input, input_len, hash_fun, output, output_max, &output_len);
    if(ret < 0)
    {
        printf("enos_hash error\n");
        return -1;
    }
    
    int ii = 0;
    printf("enos_hash input(%d):%s\n", input_len, input);
    printf("enos_hash result(%d):\n", output_len);
    for(ii = 0; ii < output_len; ii++)
    {
        printf("%02x", (unsigned char)(output[ii]));
    }
    printf("\n");
    
    return 0;
}
```

### enos_mqtt_api

<a name="connect"></a>
#### Connecting to Server

The sample code initiates connection from the gateway device (MQTT client) to the IoT Hub (MQTT server).

```
void enos_mqtt_api_connect_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

int enos_mqtt_api_connect_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_mqtt_api_connect_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_sleep(5000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}
```

In the above sample, `address` is the address of the server. If using TCP connection, the format of the server URL can be `tcp://{regionUrl}:11883`. The `gw_product_key`, `gw_device_key`, and `gw_device_secret` are device triple that you obtain from EnOS Cloud after you register the device to the cloud.

If you are not using C SDK, you can create MQTT CONNECT parameters with the device triple for other programming languages. See the following example: 

```
  mqttClientId: clientId+"|securemode=2,signmethod=hmacsha1,timestamp=132323232|"
  mqttUsername: deviceKey+"&"+productKey
  mqttPassword: uppercase(sign_hmac(deviceSecret,content))
```

Where,

- The `clientId` parameter is the client ID, which must be the same with the `clientId` in `mqttClientId`. It is recommended to use the MAC or SN of the device.
- The `timestamp` parameter can be the current time, but it must be the same with the `timestamp` in `mqttClientId`. 
- The signature should be created using the `signmethod`.

- The value of the `content` parameter is the collection of parameters (productKey, deviceKey, timestamp, and clientId) sent to the server.  The parameters and their values should be concatenated by ASCII order.
- `signmethod`: The signature generating algorithm. Supported algorithm is `hmacsha1`.
- `securemode`：The current security mode. Supported value is `2`.

For example:

```
clientId = 123, deviceKey = test, productKey = 123, timestamp = 1524448722000, deviceSecret = deviceSecret

sign= toUpperCase(hmacsha1(clientId123deviceKeytestproductKey123timestamp1524448722000deviceSecret))
```

In the above sample, the product, productKey, deviceKey, and deviceSecret can be retrieved from EnOS platform or through EnOS REST API.

<a name="connect-ssl"></a>
#### Connect to EnOS Cloud through SSL/TLS

To ensure device security, users can enable the certificate-based bi-directional authentication method through the SSL/TLS protocol. Users can apply device certificate by calling the EnOS certificate service API, load the certificate to the SDK directory, and then connect to the server through the SSL port. The server URL format is `ssl://{regionUrl}:18883`. See the code sample below.

```
void enos_mqtt_api_tls_connect_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]tls connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

int enos_mqtt_api_tls_connect_test()
{
    int ret = 0;
    char *address = "ssl://10.27.20.142:18883";
    //char *address = "beta-iot-as-mqtt-cn4.eniot.io:18883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 15000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_mqtt_api_tls_connect_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_set_ssl(enos_mas_p, "./enos_mqtt_test_tmp/ca.crt", "./enos_mqtt_test_tmp/cert.crt", "./enos_mqtt_test_tmp/csr_pri_key.pem", NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_ssl error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_sleep(10000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}
```

<a name="add-topology"></a>
#### Add Topology of Sub-devices

```
void enos_sub_dev_add_topo_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_sub_dev_add_topo_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_add_topo_asyn_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_sub_dev_add_topo_output *output = NULL;
    int ret = parse_enos_sub_dev_add_topo_data_response(data, data_len, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_sub_dev_add_topo_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, data=%s\n", output->id, output->code, output->message, output->data);
    
    free_enos_sub_dev_add_topo_output(output);
}

int enos_sub_dev_add_topo_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_sub_dev_add_topo_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    //gateway login
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    struct enos_sub_dev_add_topo_input input;
    memset(&input, 0, sizeof(struct enos_sub_dev_add_topo_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.sub_dev_count = 2;
    input.sub_dev = (struct enos_sub_dev_add_topo_input_sub_dev *)malloc(sizeof(struct enos_sub_dev_add_topo_input_sub_dev) * input.sub_dev_count);
    if(input.sub_dev == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input.sub_dev, 0, sizeof(struct enos_sub_dev_add_topo_input_sub_dev) * input.sub_dev_count);
    
    int ii = 0;
    snprintf(input.sub_dev[ii].sub_product_key, sizeof(input.sub_dev[ii].sub_product_key), "%s", "BP3X1BTv");
    snprintf(input.sub_dev[ii].sub_device_key, sizeof(input.sub_dev[ii].sub_device_key), "LSbyVxjFX9");
    snprintf(input.sub_dev[ii].sub_device_secret, sizeof(input.sub_dev[ii].sub_device_secret), "NR27DP8uI1MT8vWKunWQ");
    ii++;
    
    snprintf(input.sub_dev[ii].sub_product_key, sizeof(input.sub_dev[ii].sub_product_key), "%s", "BP3X1BTv");
    snprintf(input.sub_dev[ii].sub_device_key, sizeof(input.sub_dev[ii].sub_device_key), "q5XEil6Ndy");
    snprintf(input.sub_dev[ii].sub_device_secret, sizeof(input.sub_dev[ii].sub_device_secret), "HWmhQzMSPuXGfrwTyzdS");
    ii++;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_sub_dev_add_topo_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_add_topo_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_sub_dev_add_topo_set_callback(enos_mas_p, enos_sub_dev_add_topo_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_add_topo_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    ret = enos_sub_dev_add_topo_asyn(enos_mas_p, data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_add_topo_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    
    free(input.sub_dev);
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}
```

<a name="connect-subdevice"></a>
#### Proxy sub-devices to log into EnOS Cloud

```
void enos_sub_dev_login_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_sub_dev_login_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_login_asyn_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_sub_dev_login_output *output = NULL;
    int ret = parse_enos_sub_dev_login_data_response(data, data_len, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_sub_dev_login_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, sub_dev_count=%d\n", output->id, output->code, output->message, output->sub_dev_count);
    int ii = 0;
    for(ii = 0; ii < output->sub_dev_count; ii++)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "dev device(%d):sub_product_key=%s, sub_device_key=%s\n", \
            ii, \
            output->sub_dev[ii].sub_product_key, \
            output->sub_dev[ii].sub_device_key);
    }
    
    free_enos_sub_dev_login_output(output);
}

int enos_sub_dev_login_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_sub_dev_login_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    //gateway login
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    struct enos_sub_dev_login_input input;
    memset(&input, 0, sizeof(struct enos_sub_dev_login_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.sub_dev_count = 1;
    input.sub_dev = (struct enos_sub_dev_login_input_sub_dev *)malloc(sizeof(struct enos_sub_dev_login_input_sub_dev) * input.sub_dev_count);
    if(input.sub_dev == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]sub_dev malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    memset(input.sub_dev, 0, sizeof(struct enos_sub_dev_login_input_sub_dev) * input.sub_dev_count);
    
    int ii = 0;
    snprintf(input.sub_dev[ii].sub_product_key, sizeof(input.sub_dev[ii].sub_product_key), "%s", "BP3X1BTv");
    snprintf(input.sub_dev[ii].sub_device_key, sizeof(input.sub_dev[ii].sub_device_key), "LSbyVxjFX9");
    snprintf(input.sub_dev[ii].sub_device_secret, sizeof(input.sub_dev[ii].sub_device_secret), "NR27DP8uI1MT8vWKunWQ");
    snprintf(input.sub_dev[ii].clean_session, sizeof(input.sub_dev[ii].clean_session), "true");
    ii++;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_sub_dev_login_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_sub_dev_login_set_callback(enos_mas_p, enos_sub_dev_login_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_sub_dev_login_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    //sub_dev1 login
    ret = enos_sub_dev_login_asyn(enos_mas_p, data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    free(data);
    
    snprintf(input.id, sizeof(input.id), "%s", "2345");
    ii = 0;
    snprintf(input.sub_dev[ii].sub_product_key, sizeof(input.sub_dev[ii].sub_product_key), "%s", "BP3X1BTv");
    snprintf(input.sub_dev[ii].sub_device_key, sizeof(input.sub_dev[ii].sub_device_key), "q5XEil6Ndy");
    snprintf(input.sub_dev[ii].sub_device_secret, sizeof(input.sub_dev[ii].sub_device_secret), "HWmhQzMSPuXGfrwTyzdS");
    snprintf(input.sub_dev[ii].clean_session, sizeof(input.sub_dev[ii].clean_session), "true");
    ii++;
    
    data_len = 0;
    ret = generate_enos_sub_dev_login_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        return -1;
    }
    
    //sub_dev2 login
    ret = enos_sub_dev_login_asyn(enos_mas_p, data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_sub_dev_login_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(input.sub_dev);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);
    
    free(input.sub_dev);
    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}
```

<a name="report-measure-points"></a>
#### Report Device Measure Points

```
void enos_dev_point_post_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_point_post_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_point_post_test_reg_cb, user_str=%s, topic_name=%s(file=%s, function=%s, line=%d)\n", user_str, info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_dev_point_post_output *output = NULL;
    int ret = parse_enos_dev_point_post_data_response(data, data_len, info_ex, &output);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_point_post_data_response error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    
    free_enos_dev_point_post_output(output);
}

int enos_dev_point_post_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_point_post_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    struct enos_dev_point_post_input input;
    memset(&input, 0, sizeof(struct enos_dev_point_post_input));
    snprintf(input.id, sizeof(input.id), "%s", "1234");
    input.data = "{\
         \"enos_ai.0\": 5.55,\
         \"enos_ai.1\": 500\
         }";
    input.time_ms_is_valid = 0;
    //input.time_ms_is_valid = 1;
    //input.time_ms = 2000;
    
    char *data = NULL;
    int data_len = 0;
    ret = generate_enos_dev_point_post_data(&input, &data, &data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]generate_enos_dev_point_post_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    char *asyn_cb_user = "aaabbbccc";
    
    ret = enos_dev_point_post_set_callback(enos_mas_p, enos_dev_point_post_asyn_test_asyn_cb, asyn_cb_user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_point_post_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(data);
        return -1;
    }
    
    ret = enos_dev_point_post_asyn(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", data, data_len, NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_point_post_asyn error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        free(data);
        return -1;
    }
    
    enos_sleep(1000000);

    free(data);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}
```

<a name="invoke-service"></a>
#### Invoke Device Services(Non-Passthrough)

```
void enos_dev_service_invoke_asyn_test_cl_cb(void *user, char *cause)
{
    char *user_str = (char *)user;
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]connect lost, user_str=%s(file=%s, function=%s, line=%d)\n", user_str, __FILE__, __FUNCTION__, __LINE__);
}

void enos_dev_service_invoke_asyn_test_asyn_cb(void *user, char *data, int data_len, struct enos_mqtt_api_info_ex *info_ex)
{
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_get_event_post_test_reg_cb, topic_name=%s(file=%s, function=%s, line=%d)\n", info_ex->topic_name, __FILE__, __FUNCTION__, __LINE__);
    int data_str_max = data_len + 1;
    char *data_str = (char *)malloc(data_str_max);
    if(data_str == NULL)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]data_str malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    memset(data_str, 0, data_str_max);
    memcpy(data_str, data, data_len);
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "data_len=%d, data=%s\n\n", data_len, data_str);
    //printf("data_len=%d, data=%s\n\n", data_len, data_str);
    free(data_str);
    
    struct enos_mqtt_api_struct *enos_mas_p = (struct enos_mqtt_api_struct *)user;
    
    struct enos_dev_service_invoke_callback_data *callback_data = NULL;
    int ret = parse_enos_dev_service_invoke_callback_data(data, data_len, info_ex, &callback_data);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]parse_enos_dev_service_invoke_callback_data error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "callback_data:id=%s, product_key=%s, device_key=%s, event_id=%s, data=%s\n", callback_data->id, callback_data->info->dev_product_key, callback_data->info->dev_device_key, callback_data->info->event_id, callback_data->data);
    //printf("output:id=%s, code=%d, message=%s, product_key=%s, device_key=%s, data=%s\n", output->id, output->code, output->message, output->info->dev_product_key, output->info->dev_device_key, output->data);
    char *reply_data = NULL;
    int reply_data_len = 0;
    char *reply_data_data = "{\
                                    \"result\": 5\
                            }";
    ret = generate_enos_general_reply_data_var(callback_data->id, 200, "hello", reply_data_data, &reply_data, &reply_data_len);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]generate_enos_general_reply_data_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_service_invoke_callback_data(callback_data);
        return;
    }
    ret = enos_send_dev_service_invoke_reply_syn(enos_mas_p, callback_data->info->dev_product_key, callback_data->info->dev_device_key, callback_data->info->event_id, reply_data, reply_data_len, 0);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]generate_enos_general_reply_data_var error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free_enos_dev_service_invoke_callback_data(callback_data);
        free(reply_data);
        return;
    }
    free_enos_dev_service_invoke_callback_data(callback_data);
    free(reply_data);
    
    return;
}

int enos_dev_service_invoke_asyn_test()
{
    int ret = 0;
    char *address = "tcp://10.27.20.142:11883";
    char *gw_product_key = "rZWlw2DK";
    char *gw_device_key = "FR3V3sucNW";
    char *gw_device_secret = "3Mj3GIyq6oLqzApePCLS";
    char *cloud_charset = "UTF-8";
    char *app_charset = "UTF-8";
    char *user = "abc123";
    int timeout_ms = 5000;
    
    struct enos_mqtt_api_struct *enos_mas_p = NULL;
    ret = enos_mqtt_api_init(&enos_mas_p, address, gw_product_key, gw_device_key, gw_device_secret, cloud_charset, app_charset);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_init error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    ret = enos_mqtt_api_set_connect_lost_callback(enos_mas_p, enos_dev_service_invoke_asyn_test_cl_cb, (void *)user);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_set_connect_lost_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_mqtt_api_connect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_connect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_connect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    sub_dev_online(enos_mas_p, "5555", "BP3X1BTv", "LSbyVxjFX9", "NR27DP8uI1MT8vWKunWQ");
    enos_sleep(5000);
    
    
    ret = enos_dev_service_invoke_set_callback(enos_mas_p, enos_dev_service_invoke_asyn_test_asyn_cb, (void *)enos_mas_p);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_service_invoke_set_callback error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    ret = enos_dev_service_invoke_register(enos_mas_p, "BP3X1BTv", "LSbyVxjFX9", "enos_di.0", NULL, NULL);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_dev_service_invoke_register error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    
    enos_sleep(1000000);
    
    ret = enos_mqtt_api_disconnect(enos_mas_p, timeout_ms);
    if(ret < 0)
    {
        myprintf(NULL, ENOS_MQTT_TEST_LOG_ERROR, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        enos_mqtt_api_uninit(enos_mas_p);
        return -1;
    }
    myprintf(NULL, ENOS_MQTT_TEST_LOG_INFO, "[ENOS_MQTT_TEST]enos_mqtt_api_disconnect success(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    
    enos_mqtt_api_uninit(enos_mas_p);
    return 0;
}
```

### enos_restful_api

<a name="get-model"></a>
#### Get Thing Model

```
void restful_api_callback(void *userp,struct enos_restful_api_output *out,int cmdID)
{
	struct enos_getThingModel_data *modeldata;
	struct enos_Product_data *productdata;
	struct enos_deleteProduct_data *dproductdata;
	struct enos_applyCertificate_data *applydata;
	struct enos_listCertificates_data *listdata;
	struct enos_revokeCertificate_data *revdata;
	FILE *fp;
	
	enos_printf(NULL,ENOS_LOG_NOTHING,"cmdID=%d\n",cmdID);
	switch(cmdID)
	{
		case CMD_GET_THING_MODEL:
			
			enos_restful_api_analysis_getThingModel(out,&modeldata);
			enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",modeldata->head.status);
			enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",modeldata->head.requestId);
			enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",modeldata->head.msg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",modeldata->head.submsg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"body = %s\n",modeldata->head.body);
			enos_printf(NULL,ENOS_LOG_NOTHING,"data = %s\n",modeldata->model_data);
			enos_restful_api_free_getThingModel_data(modeldata);
			break;
	}	
	enos_restful_api_free_output(out);
}

int Test_getThingModel_asyn(struct enos_restful_api_struct *enos_restful_api)
{
	struct enos_getThingModel_input  input;
	
	sprintf(input.thingModelId,"zhangyang_dev_model_id0");		
	enos_restful_api_asyn_getThingModel(enos_restful_api,&input,restful_api_callback,NULL,CMD_GET_THING_MODEL,5);
	
	return 0;
}

int main()
{
	struct enos_restful_api_struct *my_enos_restful_api;
	char request_url[]="https://beta-portal-cn4.eniot.io:8081/enosapi";
	char access_key[]="635986bc-f6cb-4812552310a8-da8c-462a";
	char access_secret[]="552310a8-da8c-462a-a8ca-f60dbdd3a97a";
	char org_id[]="o15427722038191";

	
	enos_restful_api_init(&my_enos_restful_api);
	
	enos_restful_api_set_requestURL(my_enos_restful_api,request_url);
	
	enos_restful_api_set_accessKey(my_enos_restful_api,access_key);
	
	enos_restful_api_set_secretKey(my_enos_restful_api,access_secret);
	
	enos_restful_api_set_orgId(my_enos_restful_api,org_id);

	Test_getThingModel_asyn(my_enos_restful_api);
	
	sleep(1000);
	return 0;
}
```

<a name="apply-cert"></a>
#### Apply Certificate

```
void restful_api_callback(void *userp,struct enos_restful_api_output *out,int cmdID)
{
	struct enos_getThingModel_data *modeldata;
	struct enos_Product_data *productdata;
	struct enos_deleteProduct_data *dproductdata;
	struct enos_applyCertificate_data *applydata;
	struct enos_listCertificates_data *listdata;
	struct enos_revokeCertificate_data *revdata;
	FILE *fp;
	
	enos_printf(NULL,ENOS_LOG_NOTHING,"cmdID=%d\n",cmdID);
	switch(cmdID)
	{
		case CMD_APPLYCERT:
			
			enos_restful_api_analysis_applyCertificate(out,&applydata);
			enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",applydata->head.status);
			enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",applydata->head.requestId);
			enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",applydata->head.msg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",applydata->head.submsg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"certChainURL = %s\n",applydata->certChainURL);
			enos_printf(NULL,ENOS_LOG_NOTHING,"cert = %s\n",applydata->cert);
			enos_printf(NULL,ENOS_LOG_NOTHING,"certSN = %s\n",applydata->certSN);
			
			fp = fopen("D:\\program\\home\\CA.cert","a+");
			enos_restful_api_asyn_downloadFile(applydata->certChainURL,reatful_get_file_callBack,fp,NULL,253,10);
			enos_restful_api_free_applyCertificate_data(applydata);
			cert_step++;
			break;
		default:
			break;
	}	
	enos_restful_api_free_output(out);
}

int Test_Certificate_asyn_apply(struct enos_restful_api_struct *enos_restful_api)
{
	struct enos_applyCertificateByDeviceKey_input  input;
	char csr[]="-----BEGIN CERTIFICATE REQUEST-----\\nMIICtzCCAZ8CAQAwcjELMAkGA1UEBhMCQ04xETAPBgNVBAgMCFNoYW5naGFpMREw\\nDwYDVQQHDAhzaGFuZ2hhaTENMAsGA1UECgwERW5PUzESMBAGA1UECwwJRW5PUyBF\\nZGdlMRowGAYDVQQDDBExMjM0NTY3ODk4NzY1NDMyMTCCASIwDQYJKoZIhvcNAQEB\\nBQADggEPADCCAQoCggEBAMxfDZjhu24e1eneCbY8utEZQhdji5ep4L3131Uj9Www\\nWDfXT5BFkaWWj2aa37o7L05I/0w5UArhzTBBWHKzLouqp31S9M0a8V71Zo6TmssI\\nykxU8nfb+mbSMjZ2QqvOK7J0sJLORanjsyHl6A4eFLceqkQOPJdE9Pf4m2zgS59t\\nMNE1pCO+8kt+Goq6WD8ztXKO9gCQHwjgljjNxrtA+4XStZCD7+8Zu3LknaXdLBt/\\nrmXZgQnvSkeiTkp83+J0W7z6O/je5hbOPAVhCdm+P2ARw4+4/k5gHYdycJHkBtQh\\nMwhJBrv5TFIHQACAAQVqZTeiWIDf6aeQ5DVS9UmgYvMCAwEAAaAAMA0GCSqGSIb3\\nDQEBCwUAA4IBAQA1pesO5xg0h3J9uHlJ/C9ck3T+17Rsh3VEfWxdiRkbA5kJxTWU\\nDnvhTWMPgdsd2lKisYwhouMG0hX/sm2QN56kHYNHedDivr3rPT9HfFjGo8vQ9Ac/\\nV0jAdI2/56txjr6zJGrT/N8jF7XOinqKJFZMJjpP5jd1HzYvLWD6bQ+biFshbBqt\\nac5ma4PQ23Ag+oxHNOiJGIfqkwAHEQJccQNsjDmInrS5U7dP10ivYsAt7FRsQteQ\\nvG88+Vf+OjhtCfQsZe8OLwq/pFARjt6WJIXWPMki4ma5kEKxMDoXF0sKJcyjfmY1\\npnhVQd0PpGTqbIHvz8Bnp1hOp30ROJ+GCliP\\n-----END CERTIFICATE REQUEST-----";
	
	sprintf(input.productKey,"BP3X1BTv");
	sprintf(input.deviceKey,"LSbyVxjFX9");
	input.csr = csr;
	enos_restful_api_asyn_applyCertificateByDeviceKey(enos_restful_api,&input,restful_api_callback, NULL, CMD_APPLYCERT, 5);
	return 0;
}

int main()
{
	struct enos_restful_api_struct *my_enos_restful_api;
	char request_url[]="https://beta-portal-cn4.eniot.io:8081/enosapi";
	char access_key[]="635986bc-f6cb-4812552310a8-da8c-462a";
	char access_secret[]="552310a8-da8c-462a-a8ca-f60dbdd3a97a";
	char org_id[]="o15427722038191";

	
	enos_restful_api_init(&my_enos_restful_api);
	
	enos_restful_api_set_requestURL(my_enos_restful_api,request_url);
	
	enos_restful_api_set_accessKey(my_enos_restful_api,access_key);
	
	enos_restful_api_set_secretKey(my_enos_restful_api,access_secret);
	
	enos_restful_api_set_orgId(my_enos_restful_api,org_id);
	
	Test_Certificate_asyn_apply(my_enos_restful_api);
	
	sleep(1000);
	return 0;
}
```
<a name="project"></a>
## Project File Structure
```
├── app                    :application
├── dst                    :result dir 
├── include                :header files
├── src                    :API implementation
├── support                :third party library
├── test                   :test application and demos
├── clean_all_cmake_tmp.sh :clean up cmake intermediate files
├── CMakeLists.txt         :Cmake definition file
└── readme.md              :help
```

<a name="how-to"></a>
## Start to Use
Suppose the dst path is `/home/program/enos-api-sdk-c/dst`, the source file is `test.cpp`

```
//Include header files in this way
#ifdef __cplusplus
extern "C" {
#endif
#include "enos_c_api/enos_c_api.h"
#ifdef __cplusplus
}
#endif

//Environment variable settings
if [ $LD_LIBRARY_PATH ]
then
    export LD_LIBRARY_PATH=/home/program/enos-api-sdk-c/dst/lib/support/engines-1.1:/home/program/enos-api-sdk-c/dst/lib/support:/home/program/enos-api-sdk-c/dst/lib:$LD_LIBRARY_PATH
else
    export LD_LIBRARY_PATH=/home/program/enos-api-sdk-c/dst/lib/support/engines-1.1:/home/program/enos-api-sdk-c/dst/lib/support:/home/program/enos-api-sdk-c/dst/lib
fi

//compile
ENOS_C_API_ROOT=/home/program/enos-api-sdk-c/dst
g++ -Wno-write-strings -g -o test test.cpp -I${ENOS_C_API_ROOT}/include -I${ENOS_C_API_ROOT}/include/support -L${ENOS_C_API_ROOT}/lib -L${ENOS_C_API_ROOT}/lib/support -lenos_c_api -lcrypto -lssl -lpaho-mqtt3cs -lcjson -liconv -lcurl -lpthread
```
