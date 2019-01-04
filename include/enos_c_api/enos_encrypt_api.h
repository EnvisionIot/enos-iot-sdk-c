/*******************************************************************************
 * Copyright 2018-present Envision Digital.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 *    yang.zhang4
 *******************************************************************************/

#ifndef ENOS_ENCRYPT_API_H
#define ENOS_ENCRYPT_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "enos_common.h"

//cert version
enum ENOS_CERT_VERSION
{
    ENOS_CERT_VERSION_1 = 0,
    ENOS_CERT_VERSION_2 = 1,
    ENOS_CERT_VERSION_3 = 2
};

//cert signature algorithm
enum ENOS_CERT_SIG_ALG
{
    ENOS_CERT_SIG_ALG_sha1 = 0,
    ENOS_CERT_SIG_ALG_sha224 = 1,
    ENOS_CERT_SIG_ALG_sha256 = 2,
    ENOS_CERT_SIG_ALG_sha512_224 = 3,
    ENOS_CERT_SIG_ALG_sha512_256 = 4,
    ENOS_CERT_SIG_ALG_sha384 = 5,
    ENOS_CERT_SIG_ALG_sha512 = 6,
    ENOS_CERT_SIG_ALG_md5 = 7
};

//RSA encrypt padding mode
enum ENOS_RSA_PADDING
{
    ENOS_RSA_PADDING_PKCS1_PADDING = 0,
    ENOS_RSA_PADDING_SSLV23_PADDING = 1,
    ENOS_RSA_PADDING_NO_PADDING = 2,
    ENOS_RSA_PADDING_PKCS1_OAEP_PADDING = 3,
    ENOS_RSA_PADDING_X931_PADDING = 4,
    ENOS_RSA_PADDING_PSS_PADDING = 5
};

//default length of rsa key size(bit)
#define ENOS_DEFAULT_RSA_KEY_SIZE 2048

//default cert signature algorithm
#define ENOS_DEFAULT_SIG_ALG ENOS_CERT_SIG_ALG_sha256

//default C_country
#define ENOS_DEFAULT_REQ_C "CN"

//default ST_province
#define ENOS_DEFAULT_REQ_ST "Shanghai"

//default L_city
#define ENOS_DEFAULT_REQ_L "Shanghai"

//default O_organization
#define ENOS_DEFAULT_REQ_O "EnOS"

//default OU_organizationunit
#define ENOS_DEFAULT_REQ_OU "EnOS Edge"

//default RSA encrypt padding mode
#define ENOS_DEFAULT_RSA_PADDING ENOS_RSA_PADDING_PKCS1_PADDING

struct ENOS_C_API_DLL_EXPORT enos_cert_info
{
    int cert_version;//enum ENOS_CERT_VERSION
    int sig_alg;//enum ENOS_CERT_SIG_ALG
    char C_country[128];
    char ST_province[128];
    char L_city[128];
    char O_organization[128];
    char OU_organizationunit[128];
    char CN_commonname[128];//device SN(recommended)
};

/************************************************************
 * name:enos_generate_random_value
 * desc:generate random
 *
 * para:[out] random_ptr              result buf
 *      [in]  len                     length of random value to be generated
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_generate_random_value(char *random_ptr, int len);

/************************************************************
 * name:enos_generate_random_value_with_seed
 * desc:generate random value from a given seed
 *
 * para:[in]  seed_ptr              seed buf
 *      [in]  seed_len              length of seed
 *      [out] random_ptr            result buf
 *      [in]  len                   length of random value to be generated
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_generate_random_value_with_seed(char *seed_ptr, int seed_len, char *random_ptr, int len);

//#if defined(ENABLE_TPM)
//TPM2 operation-->
/************************************************************
 * name:eaa_generate_rsa_key_tpm2
 * desc:generate rsa public-private key(TPM2)
 *
 * para:[in]  key_path                   generated public-private key file path
 *      [in]  owner_hierarchy_passwd     TPM2 chip operation password
 *      [in]  key_passwd                 public-private key file protection password
 *      [in]  key_size                   length of rsa key size(bit)
 *      [out] pkey                       generated public-private key structure
 * return:0           success
 *        <0          fail
 * tips:1.if key_path==NULL,not generate public-private key file
 *      2.if owner_hierarchy_passwd==NULL,no operation password
 *      3.if key_passwd==NULL,no protection password
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_generate_rsa_key_tpm2(char *key_path, char *owner_hierarchy_passwd, char *key_passwd, int key_size, EVP_PKEY **pkey);

/************************************************************
 * name:enos_read_rsa_pri_key_from_file_tpm2
 * desc:loading private key from RSA public-private key file(TPM2)
 *
 * para:[in]  key_path                   public-private key file path
 *      [in]  owner_hierarchy_passwd     TPM2 chip operation password
 *      [in]  key_passwd                 public-private key file protection password
 *      [out] pri_key                    generated private key structure
 * return:0           success
 *        <0          fail
 * tips:1.if owner_hierarchy_passwd==NULL,no operation password
 *      2.if key_passwd==NULL,no protection password
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_read_rsa_pri_key_from_file_tpm2(char *key_path, char *owner_hierarchy_passwd, char *key_passwd, EVP_PKEY **pri_key);

/************************************************************
 * name:enos_read_rsa_pub_key_from_file_tpm2
 * desc:loading public key from RSA public-private key file(TPM2)
 *
 * para:[in]  key_path                   public-private key file path
 *      [in]  owner_hierarchy_passwd     TPM2 chip operation password
 *      [in]  key_passwd                 public-private key file protection password
 *      [out] pub_key                    generated public key structure
 * return:0           success
 *        <0          fail
 * tips:1.if owner_hierarchy_passwd==NULL,no operation password
 *      2.if key_passwd==NULL,no protection password
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_read_rsa_pub_key_from_file_tpm2(char *key_path, char *owner_hierarchy_passwd, char *key_passwd, EVP_PKEY **pub_key);

/************************************************************
 * name:enos_read_rsa_key_from_file_tpm2
 * desc:loading public-private key from RSA public-private key file(TPM2)
 *
 * para:[in]  key_path                   public-private key file path
 *      [in]  owner_hierarchy_passwd     TPM2 chip operation password
 *      [in]  key_passwd                 public-private key file protection password
 *      [out] pkey                       generated public-private key structure
 * return:0           success
 *        <0          fail
 * tips:1.if owner_hierarchy_passwd==NULL,no operation password
 *      2.if key_passwd==NULL,no protection password
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_read_rsa_key_from_file_tpm2(char *key_path, char *owner_hierarchy_passwd, char *key_passwd, EVP_PKEY **pkey);
//<--TPM2 operation
//#endif

/************************************************************
 * name:enos_generate_rsa_key
 * desc:generate rsa public-private key
 *
 * para:[in]  pub_key_path                   public key file path
 *      [in]  pri_key_path                   private key file path
 *      [in]  cipher_fun                     protection algorithms of private key file
 *      [in]  key_passwd                     protection password of private key file 
 *      [in]  key_size                       length of rsa key size(bit)
 *      [out] pkey                           generated public-private key structure
 * return:0           success
 *        <0          fail
 * tips:1.if pub_key_path==NULL,not generate public key file
 *      2.if pri_key_path==NULL,not generate private key file
 *      3.if cipher_fun==NULL,no protection for private key file
 *      4.possible values of cipher_fun:aes128,aes192,aes256,aria128,aria192,aria256,camellia128,camellia192,camellia256,des,des3,idea
 *      5.if pkey==NULL,not generate public-private key structure
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_generate_rsa_key(char *pub_key_path, char *pri_key_path, char *cipher_fun, char *key_passwd, int key_size, EVP_PKEY **pkey);

/************************************************************
 * name:enos_read_rsa_pri_key_from_file
 * desc:loading private key from RSA private key file
 *
 * para:[in]  key_path                   private key file path
 *      [in]  key_passwd                 private key file protection password
 *      [out] pri_key                    generated private key structure
 * return:0           success
 *        <0          fail
 * tips:1.if key_passwd==NULL,no protection password
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_read_rsa_pri_key_from_file(char *key_path, char *key_passwd, EVP_PKEY **pri_key);

/************************************************************
 * name:enos_read_rsa_pub_key_from_file
 * desc:loading public key from RSA public key file
 *
 * para:[in]  key_path                   public key file path
 *      [out] pub_key                    generated public key structure
 * return:0           success
 *        <0          fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_read_rsa_pub_key_from_file(char *key_path, EVP_PKEY **pub_key);

/************************************************************
 * name:enos_read_rsa_pub_key_from_csr
 * desc:loading public key from certificate request file
 *
 * para:[in]  csr_path                   certificate request file path
 *      [out] pub_key                    generated public key structure
 * return:0           success
 *        <0          fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_read_rsa_pub_key_from_csr(char *csr_path, EVP_PKEY **pub_key);

/************************************************************
 * name:enos_read_rsa_pub_key_from_crt_str
 * desc:loading public key from certificate string
 *
 * para:[in]  crt_str                    certificate string
 *      [in]  passwd                     certificate protection password
 *      [out] pub_key                    generated public key structure
 * return:0           success
 *        <0          fail
 * tips:1.if passwd==NULL,no protection password
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_read_rsa_pub_key_from_crt_str(char *crt_str, char *passwd, EVP_PKEY **pub_key);

/************************************************************
 * name:enos_read_rsa_pub_key_from_crt
 * desc:loading public key from certificate file
 *
 * para:[in]  crt_path                   certificate file
 *      [in]  passwd                     certificate protection password
 *      [out] pub_key                    generated public key structure
 * return:0           success
 *        <0          fail
 * tips:1.if passwd==NULL,no protection password
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_read_rsa_pub_key_from_crt(char *crt_path, char *passwd, EVP_PKEY **pub_key);

/************************************************************
 * name:enos_get_req_CN
 * desc:get CN for certificate request file
 *
 * para:[out] common_name                   result buf
 *      [out] common_name_max_size          max length of result buf
 * return:0           success
 *        <0          fail
 * tips:1.this function generate random CN, recommended CN is device SN
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_get_req_CN(char *common_name, int common_name_max_size);

/************************************************************
 * name:enos_generate_cert_req
 * desc:generate certificate request file
 *
 * para:[in] req_path                   certificate request file path
 *      [in] input_info                 information for certificate request file
 *      [in] pkey                       public-private key structure for certificate request file
 * return:0           success
 *        <0          fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_generate_cert_req(char *req_path, struct enos_cert_info *input_info, EVP_PKEY *pkey);

/************************************************************
 * name:enos_crt_sign_verify
 * desc:verify certificate
 *
 * para:[in] signed_crt_path                   certificate to be certified
 *      [in] signed_crt_passwd                 protection passwd of certificate to be certified
 *      [in] root_crt_path                     root certificate path
 *      [in] root_crt_passwd                   protection passwd of root certificate
 *      [in] crl_path                          certificate revocation list
 *      [in] crl_passwd                        protection passwd of certificate revocation list
 * return:0           success
 *        <0          fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_crt_sign_verify(char *signed_crt_path, char *signed_crt_passwd, char *root_crt_path, char *root_crt_passwd, char *crl_path, char *crl_passwd);

/************************************************************
 * name:enos_rsa_public_encrypt
 * desc:RSA public key encryption
 *
 * para:[in]  pkey                   public key structure
 *      [in]  input                  data to be encrypted
 *      [in]  input_len              length of input
 *      [out] output                 result buf
 *      [in]  output_max             max length of output
 *      [out] output_len             actual length of output
 *      [in]  padding_mode           padding mode
 * return:0           success
 *        <0          fail
 * tips:1.if result length > output_max, return fail
 *      2.padding_mode is defined by enum ENOS_RSA_PADDING, EAA_RSA_PADDING_PKCS1_PADDING and EAA_RSA_PADDING_NO_PADDING are supported now
 *      3.if padding_mode==EAA_RSA_PADDING_PKCS1_PADDING,input_len should <= rsa_key_size(bit)/8-11, or it will return fail
 *      4.if padding_mode==EAA_RSA_PADDING_NO_PADDING,input_len should == rsa_key_size(bit)/8, or it will return fail
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_rsa_public_encrypt(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len, int padding_mode);

/************************************************************
 * name:enos_rsa_private_decrypt
 * desc:RSA private key decryption
 *
 * para:[in]  pkey                   private key structure
 *      [in]  input                  data to be decrypted
 *      [in]  input_len              length of input
 *      [out] output                 result buf
 *      [in]  output_max             max length of output
 *      [out] output_len             actual length of output
 *      [in]  padding_mode           padding mode
 * return:0           success
 *        <0          fail
 * tips:1.if result length > output_max, return fail
 *      2.padding_mode is defined by enum ENOS_RSA_PADDING, EAA_RSA_PADDING_PKCS1_PADDING and EAA_RSA_PADDING_NO_PADDING are supported now
 *      3.input_len should == rsa_key_size(bit)/8, or it will return fail
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_rsa_private_decrypt(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len, int padding_mode);

/************************************************************
 * name:enos_rsa_public_encrypt_advance
 * desc:RSA public key encryption(support arbitrary length input)
 *
 * para:[in]  pkey                   public key structure
 *      [in]  input                  data to be encrypted
 *      [in]  input_len              length of input
 *      [out] output                 result buf
 *      [in]  output_max             max length of output
 *      [out] output_len             actual length of output
 * return:0           success
 *        <0          fail
 * tips:1.if result length > output_max, return fail
 *      2.using EAA_RSA_PADDING_PKCS1_PADDING padding mode inside the function
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_rsa_public_encrypt_advance(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len);

/************************************************************
 * name:enos_rsa_private_decrypt_advance
 * desc:RSA private key decryption(decrypt for enos_rsa_public_encrypt_advance)
 *
 * para:[in]  pkey                   private key structure
 *      [in]  input                  data to be decrypted
 *      [in]  input_len              length of input
 *      [out] output                 result buf
 *      [in]  output_max             max length of output
 *      [out] output_len             actual length of output
 * return:0           success
 *        <0          fail
 * tips:1.if result length > output_max, return fail
 *      2.decrypt for enos_rsa_public_encrypt_advance only
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_rsa_private_decrypt_advance(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len);

/************************************************************
 * name:enos_rsa_public_encrypt_file
 * desc:RSA public key encryption for file
 *
 * para:[in]  pkey                   public key structure
 *      [in]  file_in                file to be encrypted
 *      [in]  file_out               result file
 * return:0           success
 *        <0          fail
 * tips:1.using EAA_RSA_PADDING_PKCS1_PADDING padding mode inside the function
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_rsa_public_encrypt_file(EVP_PKEY *pkey, char *file_in, char *file_out);

/************************************************************
 * name:enos_rsa_private_decrypt_file
 * desc:RSA private key decryption for file
 *
 * para:[in]  pkey                   private key structure
 *      [in]  file_in                file to be decrypted
 *      [in]  file_out               result file
 * return:0           success
 *        <0          fail
 * tips:1.decrypt for enos_rsa_public_encrypt_file only
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_rsa_private_decrypt_file(EVP_PKEY *pkey, char *file_in, char *file_out);

/************************************************************
 * name:enos_rsa_private_encrypt
 * desc:RSA private key encryption
 *
 * para:[in]  pkey                   private key structure
 *      [in]  input                  data to be encrypted
 *      [in]  input_len              length of input
 *      [out] output                 result buf
 *      [in]  output_max             max length of output
 *      [out] output_len             actual length of output
 *      [in]  padding_mode           padding mode
 * return:0           success
 *        <0          fail
 * tips:1.if result length > output_max, return fail
 *      2.padding_mode is defined by enum ENOS_RSA_PADDING, EAA_RSA_PADDING_PKCS1_PADDING and EAA_RSA_PADDING_NO_PADDING are supported now
 *      3.if padding_mode==EAA_RSA_PADDING_PKCS1_PADDING,input_len should <= rsa_key_size(bit)/8-11, or it will return fail
 *      4.if padding_mode==EAA_RSA_PADDING_NO_PADDING,input_len should == rsa_key_size(bit)/8, or it will return fail
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_rsa_private_encrypt(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len, int padding_mode);

/************************************************************
 * name:enos_rsa_public_decrypt
 * desc:RSA public key decryption
 *
 * para:[in]  pkey                   public key structure
 *      [in]  input                  data to be decrypted
 *      [in]  input_len              length of input
 *      [out] output                 result buf
 *      [in]  output_max             max length of output
 *      [out] output_len             actual length of output
 *      [in]  padding_mode           padding mode
 * return:0           success
 *        <0          fail
 * tips:1.if result length > output_max, return fail
 *      2.padding_mode is defined by enum ENOS_RSA_PADDING, EAA_RSA_PADDING_PKCS1_PADDING and EAA_RSA_PADDING_NO_PADDING are supported now
 *      3.input_len should == rsa_key_size(bit)/8, or it will return fail
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_rsa_public_decrypt(EVP_PKEY *pkey, char *input, int input_len, char *output, int output_max, int *output_len, int padding_mode);

/************************************************************
 * name:enos_hash
 * desc:hash
 *
 * para:[in]  input                  data to be hash
 *      [in]  input_len              length of input
 *      [in]  hash_fun               hash function
 *      [out] output                 result buf
 *      [in]  output_max             max length of output
 *      [out] output_len             actual length of output
 * return:0           success
 *        <0          fail
 * tips:1.if result length > output_max, return fail
 *      2.possible values of hash_fun:
 *        blake2b512        blake2s256        gost              md4               
 *        md5               mdc2              rmd160            sha1              
 *        sha224            sha256            sha3-224          sha3-256          
 *        sha3-384          sha3-512          sha384            sha512            
 *        sha512-224        sha512-256        shake128          shake256          
 *        sm3
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_hash(char *input, int input_len, char *hash_fun, char *output, int output_max, int *output_len);

#ifdef __cplusplus
}
#endif

#endif
