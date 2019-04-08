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
 *    renbin.wang
 *******************************************************************************/

#ifndef ENOS_RESTFUL_API_H
#define ENOS_RESTFUL_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "enos_common.h"
#include "curl/curl.h"


//define request url string max len
#define REQUEST_URL_MAX_LEN  10240


//define restful api struct
struct ENOS_C_API_DLL_EXPORT enos_restful_api_struct
{
    char requestURL[ENOS_GENERAL_BUF_MAX];   //HTTPS URL
		char accessKey[ENOS_GENERAL_BUF_MAX];    //access_key
		char secretKey[ENOS_GENERAL_BUF_MAX];    //access_secret
    char orgId[ENOS_GENERAL_BUF_MAX];        //orgId
};

//define the struct of the return buffer form URL server,do not forget call the function "enos_restful_api_free_output" to free memory when the struct is no longer used
struct ENOS_C_API_DLL_EXPORT enos_restful_api_output
{
	      int len;        //length of data
       char *data;      //pointer of data
   CURLcode url_code;  //libcurl error code
};

//define output head struct,make sure to check the pointer is null or not before it is used
struct ENOS_C_API_DLL_EXPORT enos_restful_api_output_head
{
	int  status;
	char *requestId;
	char *msg;
	char *submsg;
	char *body;
};

//define asynchronously opertor struct
//define callback function and parameter struct
typedef void (*enos_restful_api_callback)(void *, struct enos_restful_api_output *, int);
struct ENOS_C_API_DLL_EXPORT enos_url_asyn_para_struct
{
	enos_restful_api_callback cb;
	char request_url[REQUEST_URL_MAX_LEN];
	char *body;
	void *user;
	int  cmdID;
	int timeout;
};
//Comm function------>
/************************************************************
 * name:enos_restful_api_init
 * desc:init the struct before use restful api
 *
 * para:[in] enos_ras_pp     pointer of the pointer of enos_restful_api_struct
 * return:0         success
 *        <0        fail
 * tips:1.the memory of enos_ras_pp is malloced by the interface 
 *				library,you should call the function to free enos_ras_pp 
 *				when it is unused
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_init(struct enos_restful_api_struct **enos_ras_pp);

/************************************************************
 * name:enos_restful_api_uninit
 * desc:to free the memory after restful api used
 *
 * para:[in] enos_ras_pp     pointer of enos_restful_api_struct
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_uninit(struct enos_restful_api_struct *enos_ras_p);

/************************************************************
 * name:enos_restful_api_set_requestURL
 * desc:set the requestURL in the struct of enos_restful_api_struct
 *
 * para:[in] enos_ras_pp     pointer of enos_restful_api_struct
 * para:[in] requestURL     requestURL buffer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_set_requestURL(struct enos_restful_api_struct *enos_ras_p,char *requestURL);

/************************************************************
 * name:enos_restful_api_set_accessKey
 * desc:set the accessKey in the struct of enos_restful_api_struct
 *
 * para:[in] enos_ras_pp     pointer of enos_restful_api_struct
 * para:[in] accessKey     accessKey buffer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_set_accessKey(struct enos_restful_api_struct *enos_ras_p,char *accessKey);

/************************************************************
 * name:enos_restful_api_set_secretKey
 * desc:set the secretKey in the struct of enos_restful_api_struct
 *
 * para:[in] enos_ras_pp     pointer of enos_restful_api_struct
 * para:[in] secretKey     secretKey buffer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_set_secretKey(struct enos_restful_api_struct *enos_ras_p,char *secretKey);

/************************************************************
 * name:enos_restful_api_set_orgId
 * desc:set the orgId in the struct of enos_restful_api_struct
 *
 * para:[in] enos_ras_pp     pointer of enos_restful_api_struct
 * para:[in] orgId     orgId buffer
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_set_orgId(struct enos_restful_api_struct *enos_ras_p,char *orgId);

/************************************************************
 * name:enos_restful_api_free_output
 * desc:free memory of the struct enos_restful_api_output
 *
 * para:[in] output     pointer of enos_restful_api_output
 * return:0         success
 *        <0        fail
 * tips:1.the memory of the return buffer from url server is 
 					malloced by the interface library,you should free it
 					after used
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_free_output(struct enos_restful_api_output *output);

/************************************************************
 * name:enos_calc_sign_restful
 * desc:calculate the sign in the request url
 *
 * para:[in] para_value_p  an array of struct enos_para_value
 * para:[in] count     		the number of entries in the para_value_p array
 * para:[in] accessKey    accessKey buffer
 * para:[in] secretKey    secretKey buffer
 * para:[out] sign     		sign result buffer
 * para:[in] sign_max     max length of sign result
 * para:[out] sign_len    ength of sign result
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
//ENOS_C_API_DLL_EXPORT extern int enos_calc_sign_restful(struct enos_para_value *para_value_p, int count, char *accessKey,char *secretKey, char *sign, int sign_max, int *sign_len);

/************************************************************
 * name:enos_calc_sign_with_body_restful
 * desc:calculate the sign in the request url
 *
 * para:[in] para_value_p  an array of struct enos_para_value
 * para:[in] count     		the number of entries in the para_value_p array
 * para:[in] body     	  body ptr, can be NULL
 * para:[in] accessKey    accessKey buffer
 * para:[in] secretKey    secretKey buffer
 * para:[out] sign     		sign result buffer
 * para:[in] sign_max     max length of sign result
 * para:[out] sign_len    ength of sign result
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_calc_sign_with_body_restful(struct enos_para_value *para_value_p, int count, char *body, char *accessKey,char *secretKey, char *sign, int sign_max, int *sign_len);
//<------Comm function



//------->function about ThingModel
/*
 * getThingModel
 */
//define intput strcut of getThingModel
struct ENOS_C_API_DLL_EXPORT enos_getThingModel_input
{
    char thingModelId[ENOS_GENERAL_BUF_MAX];
};
//define data strcut of getThingModel
struct ENOS_C_API_DLL_EXPORT enos_getThingModel_data
{
    struct enos_restful_api_output_head head;
      char *model_data;
};

/************************************************************
 * name:enos_restful_api_syn_getThingModel
 * desc:getThingModel synchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[out] output    buffer returned by url server
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_getThingModel(struct enos_restful_api_struct *enos_ras_p, struct enos_getThingModel_input *input, struct enos_restful_api_output **output, int timeout);
/************************************************************
 * name:enos_restful_api_asyn_getThingModel
 * desc:getThingModel asynchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[in] cb    	callback function
 * para:[in] user   user pointer
 * para:[in] cmdID  url request cmdID,defined by user,used in callback function
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_getThingModel(struct enos_restful_api_struct *enos_ras_p, struct enos_getThingModel_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout);
/************************************************************
 * name:enos_restful_api_analysis_getThingModel
 * desc:analysis getThingModel url return buffer
 *
 * para:[in] output  buffer returned by url server
 * para:[out]  data  pointer of struct enos_getThingModel_data pointer
 * return:0         success
 *        <0        fail
 * tips:data should be free when it is unused
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_analysis_getThingModel(struct enos_restful_api_output *output, struct enos_getThingModel_data **data);
/************************************************************
 * name:enos_restful_api_free_getThingModel_data
 * desc:free memory of struct enos_getThingModel_data
 *
 * para:[int]  data  pointer of struct enos_getThingModel_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_free_getThingModel_data(struct ENOS_C_API_DLL_EXPORT enos_getThingModel_data *data);
//<--------function about ThingModel


//------->function about Product
/*
 * getProduct
 */
//define intput strcut of getProduct
struct ENOS_C_API_DLL_EXPORT enos_getProduct_input
{
    char productKey[ENOS_GENERAL_BUF_MAX];
};

/************************************************************
 * name:enos_restful_api_syn_getProduct
 * desc:getProduct synchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[out] output    buffer returned by url server
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_getProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_getProduct_input *input, struct enos_restful_api_output **output, int timeout);
/************************************************************
 * name:enos_restful_api_asyn_getProduct
 * desc:getProduct asynchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[in] cb    	callback function
 * para:[in] user   user pointer
 * para:[in] cmdID  url request cmdID,defined by user,used in callback function
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_getProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_getProduct_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout);

/*
 * createProduct
 */
//define intput strcut of createProduct
struct ENOS_C_API_DLL_EXPORT enos_createProduct_input
{
	char productName[ENOS_GENERAL_BUF_MAX];
	char productDesc[ENOS_GENERAL_BUF_MAX];
	char modelId[ENOS_GENERAL_BUF_MAX];
	int  dataType;
	int  nodeType;
	int  authType;
	int  optional_flag;//set the flag of optional para used in the url. 
										//bit0:dataType;
										//bit1:nodeType;
										//bit2:authType
};

/************************************************************
 * name:enos_restful_api_syn_createProduct
 * desc:createProduct synchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[out] output    buffer returned by url server
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_createProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_createProduct_input *input, struct enos_restful_api_output **output, int timeout);
/************************************************************
 * name:enos_restful_api_asyn_createProduct
 * desc:createProduct asynchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[in] cb    	callback function
 * para:[in] user   user pointer
 * para:[in] cmdID  url request cmdID,defined by user,used in callback function
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_createProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_createProduct_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout);

/*
 * updateProduct
 */
//define intput strcut of updateProduct
struct ENOS_C_API_DLL_EXPORT enos_updateProduct_input
{
	char productKey[ENOS_GENERAL_BUF_MAX];
  char productName[ENOS_GENERAL_BUF_MAX];
	char productDesc[ENOS_GENERAL_BUF_MAX];
	int  dynamic;
};
/************************************************************
 * name:enos_restful_api_syn_updateProduct
 * desc:updateProduct synchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[out] output    buffer returned by url server
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_updateProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_updateProduct_input *input, struct enos_restful_api_output **output, int timeout);
/************************************************************
 * name:enos_restful_api_asyn_updateProduct
 * desc:updateProduct asynchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[in] cb    	callback function
 * para:[in] user   user pointer
 * para:[in] cmdID  url request cmdID,defined by user,used in callback function
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_updateProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_updateProduct_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout);

/*
 * Product data analysis
 */
//define data strcut of Product
struct ENOS_C_API_DLL_EXPORT enos_Product_data
{
    struct enos_restful_api_output_head head;
      char *productId;
      char *productKey;
      char *productName;
      char *productSecret;
      char *productDesc;
      int  dataType;
      int  nodeType;
      int  authType;
      char *productTags;
      char *modelId;
      int  dynamic;
};

/************************************************************
 * name:enos_restful_api_analysis_Product
 * desc:analysis url return buffer of getProduct/createProduct/updateProduct
 *
 * para:[in] output  buffer returned by url server
 * para:[out]  data  pointer of struct enos_Product_data pointer
 * return:0         success
 *        <0        fail
 * tips:data should be free when it is unused
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_analysis_Product(struct enos_restful_api_output *output, struct enos_Product_data **data);
/************************************************************
 * name:enos_restful_api_free_Product_data
 * desc:free memory of struct enos_Product_data
 *
 * para:[int]  data  pointer of struct enos_Product_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_free_Product_data(struct ENOS_C_API_DLL_EXPORT enos_Product_data *data);


/*
 * deleteProduct
 */
//define intput strcut of deleteProduct
struct ENOS_C_API_DLL_EXPORT enos_deleteProduct_input
{
    char productKey[ENOS_GENERAL_BUF_MAX];
};
//define data strcut of deleteProduct
struct ENOS_C_API_DLL_EXPORT enos_deleteProduct_data
{
    struct enos_restful_api_output_head head;
      char *data;
};
/************************************************************
 * name:enos_restful_api_syn_deleteProduct
 * desc:deleteProduct synchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[out] output    buffer returned by url server
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_deleteProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_deleteProduct_input *input, struct enos_restful_api_output **output, int timeout);
/************************************************************
 * name:enos_restful_api_asyn_deleteProduct
 * desc:deleteProduct asynchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[in] cb    	callback function
 * para:[in] user   user pointer
 * para:[in] cmdID  url request cmdID,defined by user,used in callback function
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_deleteProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_deleteProduct_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout);
/************************************************************
 * name:enos_restful_api_analysis_deleteProduct
 * desc:analysis deleteProduct url return buffer
 *
 * para:[in] output  buffer returned by url server
 * para:[out]  data  pointer of struct enos_deleteProduct_data pointer
 * return:0         success
 *        <0        fail
 * tips:data should be free when it is unused
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_analysis_deleteProduct(struct enos_restful_api_output *output, struct enos_deleteProduct_data **data);
/************************************************************
 * name:enos_restful_api_free_deleteProduct_data
 * desc:free memory of struct enos_deleteProduct_data
 *
 * para:[int]  data  pointer of struct enos_deleteProduct_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_free_deleteProduct_data(struct ENOS_C_API_DLL_EXPORT enos_deleteProduct_data *data);

//<-------function about Product

//------->function about Certificate
/*
 * applyCertificateByAssetId
 */
//define intput strcut of applyCertificateByAssetId
struct ENOS_C_API_DLL_EXPORT enos_applyCertificateByAssetId_input
{
	char assetId[ENOS_GENERAL_BUF_MAX];
  char *csr;
};
/************************************************************
 * name:enos_restful_api_syn_applyCertificateByAssetId
 * desc:applyCertificateByAssetId synchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[out] output    buffer returned by url server
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_applyCertificateByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_applyCertificateByAssetId_input *input, struct enos_restful_api_output **output, int timeout);
/************************************************************
 * name:enos_restful_api_asyn_applyCertificateByAssetId
 * desc:applyCertificateByAssetId asynchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[in] cb    	callback function
 * para:[in] user   user pointer
 * para:[in] cmdID  url request cmdID,defined by user,used in callback function
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_applyCertificateByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_applyCertificateByAssetId_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout);

/*
 * applyCertificateByDeviceKey
 */
//define intput strcut of applyCertificateByDeviceKey
struct ENOS_C_API_DLL_EXPORT enos_applyCertificateByDeviceKey_input
{
	char productKey[ENOS_GENERAL_BUF_MAX];
	char deviceKey[ENOS_GENERAL_BUF_MAX];
  char *csr;
};

/************************************************************
 * name:enos_restful_api_syn_applyCertificateByDeviceKey
 * desc:applyCertificateByDeviceKey synchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[out] output    buffer returned by url server
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_applyCertificateByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_applyCertificateByDeviceKey_input *input, struct enos_restful_api_output **output, int timeout);
/************************************************************
 * name:enos_restful_api_asyn_applyCertificateByDeviceKey
 * desc:applyCertificateByDeviceKey asynchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[in] cb    	callback function
 * para:[in] user   user pointer
 * para:[in] cmdID  url request cmdID,defined by user,used in callback function
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_applyCertificateByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_applyCertificateByDeviceKey_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout);

/*
 * applyCertificate analysis
 */
//define data strcut of applyCertificate
struct ENOS_C_API_DLL_EXPORT enos_applyCertificate_data
{
	struct enos_restful_api_output_head head;
  char *certChainURL;
  char *cert;
  char *certSN;
};
/************************************************************
 * name:enos_restful_api_analysis_applyCertificate
 * desc:analysis applyCertificate url return buffer
 *
 * para:[in] output  buffer returned by url server
 * para:[out]  data  pointer of struct enos_applyCertificate_data pointer
 * return:0         success
 *        <0        fail
 * tips:data should be free when it is unused
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_analysis_applyCertificate(struct enos_restful_api_output *output, struct enos_applyCertificate_data **data);
/************************************************************
 * name:enos_restful_api_free_applyCertificate_data
 * desc:free memory of struct enos_applyCertificate_data
 *
 * para:[int]  data  pointer of struct enos_applyCertificate_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_free_applyCertificate_data(struct ENOS_C_API_DLL_EXPORT enos_applyCertificate_data *data);



/*
 * listCertificatesByAssetId
 */
//define intput strcut of listCertificatesByAssetId
struct ENOS_C_API_DLL_EXPORT enos_listCertificatesByAssetId_input
{
	char assetId[ENOS_GENERAL_BUF_MAX];
};
/************************************************************
 * name:enos_restful_api_syn_listCertificatesByAssetId
 * desc:listCertificatesByAssetId synchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[out] output    buffer returned by url server
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_listCertificatesByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_listCertificatesByAssetId_input *input, struct enos_restful_api_output **output, int timeout);
/************************************************************
 * name:enos_restful_api_asyn_listCertificatesByAssetId
 * desc:listCertificatesByAssetId asynchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[in] cb    	callback function
 * para:[in] user   user pointer
 * para:[in] cmdID  url request cmdID,defined by user,used in callback function
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_listCertificatesByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_listCertificatesByAssetId_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout);


/*
 * listCertificatesByDeviceKey
 */
//define intput strcut of listCertificatesByDeviceKey
struct ENOS_C_API_DLL_EXPORT enos_listCertificatesByDeviceKey_input
{
	char productKey[ENOS_GENERAL_BUF_MAX];
	char deviceKey[ENOS_GENERAL_BUF_MAX];
};
/************************************************************
 * name:enos_restful_api_syn_listCertificatesByDeviceKey
 * desc:listCertificatesByDeviceKey synchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[out] output    buffer returned by url server
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_listCertificatesByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_listCertificatesByDeviceKey_input *input, struct enos_restful_api_output **output, int timeout);
/************************************************************
 * name:enos_restful_api_asyn_listCertificatesByDeviceKey
 * desc:getThingModel listCertificatesByDeviceKey
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[in] cb    	callback function
 * para:[in] user   user pointer
 * para:[in] cmdID  url request cmdID,defined by user,used in callback function
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_listCertificatesByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_listCertificatesByDeviceKey_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout);

/*
 * listCertificate analysis
 */
//define strcut of certData
struct ENOS_C_API_DLL_EXPORT enos_listCertificates_certData
{
	char *certSN;
	int certStatus;
};
//define data strcut of listCertificate
struct ENOS_C_API_DLL_EXPORT enos_listCertificates_data
{
	struct enos_restful_api_output_head head;
	struct enos_listCertificates_certData *certData;
	int cert_num;
};
/************************************************************
 * name:enos_restful_api_analysis_listCertificates
 * desc:analysis listCertificates url return buffer
 *
 * para:[in] output  buffer returned by url server
 * para:[out]  data  pointer of struct enos_listCertificates_data pointer
 * return:0         success
 *        <0        fail
 * tips:data should be free when it is unused
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_analysis_listCertificates(struct enos_restful_api_output *output, struct enos_listCertificates_data **data);
/************************************************************
 * name:enos_restful_api_free_listCertificates_data
 * desc:free memory of struct enos_listCertificates_data
 *
 * para:[int]  data  pointer of struct enos_listCertificates_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_free_listCertificates_data(struct ENOS_C_API_DLL_EXPORT enos_listCertificates_data *data);


/*
 * revokeCertificateByAssetId
 */
//define intput strcut of revokeCertificateByAssetId
struct ENOS_C_API_DLL_EXPORT enos_revokeCertificateByAssetId_input
{
	char assetId[ENOS_GENERAL_BUF_MAX];
	char certSN[ENOS_GENERAL_BUF_MAX];
	int  optional_flag; //set the flag of optional para used in the url.
											//bit0:certSN
};
/************************************************************
 * name:enos_restful_api_syn_revokeCertificateByAssetId
 * desc:revokeCertificateByAssetId synchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[out] output    buffer returned by url server
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_revokeCertificateByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_revokeCertificateByAssetId_input *input, struct enos_restful_api_output **output, int timeout);
/************************************************************
 * name:enos_restful_api_asyn_revokeCertificateByAssetId
 * desc:revokeCertificateByAssetId asynchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[in] cb    	callback function
 * para:[in] user   user pointer
 * para:[in] cmdID  url request cmdID,defined by user,used in callback function
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_revokeCertificateByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_revokeCertificateByAssetId_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout);


/*
 * revokeCertificateByDeviceKey
 */
//define intput strcut of revokeCertificateByDeviceKey
struct ENOS_C_API_DLL_EXPORT enos_revokeCertificateByDeviceKey_input
{
	char productKey[ENOS_GENERAL_BUF_MAX];
	char deviceKey[ENOS_GENERAL_BUF_MAX];
	char certSN[ENOS_GENERAL_BUF_MAX];
	int  optional_flag; //set the flag of optional para used in the url.
											//bit0:certSN
};
/************************************************************
 * name:enos_restful_api_syn_revokeCertificateByDeviceKey
 * desc:revokeCertificateByDeviceKey synchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[out] output    buffer returned by url server
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_revokeCertificateByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_revokeCertificateByDeviceKey_input *input, struct enos_restful_api_output **output, int timeout);
/************************************************************
 * name:enos_restful_api_asyn_revokeCertificateByDeviceKey
 * desc:revokeCertificateByDeviceKey asynchronously
 *
 * para:[in] enos_ras_p  poniter of struct enos_restful_api_struct
 * para:[in]  input     intput information
 * para:[in] cb    	callback function
 * para:[in] user   user pointer
 * para:[in] cmdID  url request cmdID,defined by user,used in callback function
 * para:[in]  timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_revokeCertificateByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_revokeCertificateByDeviceKey_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout);

/*
 * revokeCertificate analysis
 */
//define data strcut of revokeCertificate
struct ENOS_C_API_DLL_EXPORT enos_revokeCertificate_data
{
	struct enos_restful_api_output_head head;
	  char *data;
};
/************************************************************
 * name:enos_restful_api_analysis_revokeCertificate
 * desc:analysis revokeCertificate url return buffer
 *
 * para:[in] output  buffer returned by url server
 * para:[out]  data  pointer of struct enos_revokeCertificate_data pointer
 * return:0         success
 *        <0        fail
 * tips:data should be free when it is unused
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_analysis_revokeCertificate(struct enos_restful_api_output *output, struct enos_revokeCertificate_data **data);
/************************************************************
 * name:enos_restful_api_free_revokeCertificate_data
 * desc:free memory of struct enos_revokeCertificate_data
 *
 * para:[int]  data  pointer of struct enos_revokeCertificate_data
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_free_revokeCertificate_data(struct ENOS_C_API_DLL_EXPORT enos_revokeCertificate_data *data);

//<-------function about Certificate

//------->function about upload file
/************************************************************
 * name:enos_restful_api_syn_downloadFile
 * desc:downloadFile synchronously
 *
 * para:[in] request_url  downloadFile url string
 * para:[in]  fp          file pointer used to save file,the pointer 
 *                        should be opened with the attribute of "a+"
 * para:[in]  timeout     time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_syn_downloadFile(char *request_url,FILE *fp,int timeout);
//define callback function used by dowmloadFile asynchronously
typedef void (*enos_restful_api_dowmloadFile_callback)(void *,FILE *,int);
//define parameter struct used by dowmloadFile asynchronously
struct ENOS_C_API_DLL_EXPORT enos_url_asyn_dowmloadFile_para_struct
{
	enos_restful_api_dowmloadFile_callback cb;
	char request_url[REQUEST_URL_MAX_LEN];
	FILE *fp;
	int timeout;
	void *user;
	int  cmdID;
};
/************************************************************
 * name:enos_restful_api_asyn_downloadFile
 * desc:downloadFile asynchronously
 *
 * para:[in] request_url  downloadFile url string
 * para:[in] cb     		callback function
 * para:[in] fp   		 file pointer used to save file,the pointer 
 *                     should be opened with the attribute of "a+"
 * para:[in] user      user pointer
 * para:[out] cmdID    url request cmdID,defined by user,used in callback function
 * para:[in] timeout   time out of url request
 * return:0         success
 *        <0        fail
 * tips:
 ************************************************************/
ENOS_C_API_DLL_EXPORT extern int enos_restful_api_asyn_downloadFile(char *request_url,enos_restful_api_dowmloadFile_callback cb,FILE *fp,void *user,int  cmdID,int timeout);
//<-------function about upload file
#ifdef __cplusplus
}
#endif

#endif
