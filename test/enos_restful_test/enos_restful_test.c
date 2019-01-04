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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "enos_c_api/enos_c_api.h"

//define cmdID
enum{CMD_NULL=0,CMD_GET_THING_MODEL,CMD_CREAT_PRODUCT,CMD_GET_PRODUCT,CMD_UPDATE_PRODUCT,CMD_DELETE_PRODUCT,CMD_APPLYCERT,CMD_LISTCERT,CMD_DELETECERT};
//define step of asynchronous request about protuct
enum{PWAIT_CREAT=0,PCREAT,PWAIT_GET,PGET,PWAIT_UPDATE,PUPDATE,PWAIT_DELETE,PDELETE};
//define step of asynchronous request about cert
enum{CWAIT_APPLY=0,CAPPLY,CWAIT_LIST,CLIST,CWAIT_DELETE,CDELETE};
char asyn_productKey[ENOS_GENERAL_BUF_MAX];
char product_step=0;
char cert_step=0;

void reatful_get_file_callBack(void *user,FILE *fp, int cmdID);
void restful_api_callback(void *userp,struct enos_restful_api_output *out,int cmdID);
int Test_getThingModel_syn(struct enos_restful_api_struct *enos_restful_api);
int Test_getThingModel_asyn(struct enos_restful_api_struct *enos_restful_api);
int Test_product_syn(struct enos_restful_api_struct *enos_restful_api);
int Test_product_asyn_creat(struct enos_restful_api_struct *enos_restful_api);
int Test_product_asyn_get(struct enos_restful_api_struct *enos_restful_api);
int Test_product_asyn_update(struct enos_restful_api_struct *enos_restful_api);
int Test_product_asyn_delete(struct enos_restful_api_struct *enos_restful_api);

int Test_Certificate_syn(struct enos_restful_api_struct *enos_restful_api);
int Test_Certificate_asyn_apply(struct enos_restful_api_struct *enos_restful_api);
int Test_Certificate_asyn_list(struct enos_restful_api_struct *enos_restful_api);
int Test_Certificate_asyn_delete(struct enos_restful_api_struct *enos_restful_api);
//callback of asynchronous download file
void reatful_get_file_callBack(void *user,FILE *fp, int cmdID)
{
	enos_printf(NULL,ENOS_LOG_NOTHING,"cmdID=%d\n",cmdID);
	fclose(fp);
}
//callback of asynchronous request
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
		case CMD_CREAT_PRODUCT:
		case CMD_GET_PRODUCT:
		case CMD_UPDATE_PRODUCT:
			enos_restful_api_analysis_Product(out,&productdata);
			enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",productdata->head.status);
			enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",productdata->head.requestId);
			enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",productdata->head.msg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",productdata->head.submsg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"body = %s\n",productdata->head.body);
			enos_printf(NULL,ENOS_LOG_NOTHING,"productId = %s\n",productdata->productId);
			enos_printf(NULL,ENOS_LOG_NOTHING,"productKey = %s\n",productdata->productKey);
			enos_printf(NULL,ENOS_LOG_NOTHING,"productName = %s\n",productdata->productName);
			enos_printf(NULL,ENOS_LOG_NOTHING,"productSecret = %s\n",productdata->productSecret);
			enos_printf(NULL,ENOS_LOG_NOTHING,"productDesc = %s\n",productdata->productDesc);
			enos_printf(NULL,ENOS_LOG_NOTHING,"dataType = %d\n",productdata->dataType);
			enos_printf(NULL,ENOS_LOG_NOTHING,"nodeType = %d\n",productdata->nodeType);
			enos_printf(NULL,ENOS_LOG_NOTHING,"productTags = %s\n",productdata->productTags);
			enos_printf(NULL,ENOS_LOG_NOTHING,"modelId = %s\n",productdata->modelId);
			enos_printf(NULL,ENOS_LOG_NOTHING,"dynamic = %d\n",productdata->dynamic);
			sprintf(asyn_productKey,productdata->productKey);
			enos_restful_api_free_Product_data(productdata);
			product_step++;
			break;
		case CMD_DELETE_PRODUCT:
			
			enos_restful_api_analysis_deleteProduct(out,&dproductdata);
			enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",dproductdata->head.status);
			enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",dproductdata->head.requestId);
			enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",dproductdata->head.msg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",dproductdata->head.submsg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"body = %s\n",dproductdata->head.body);
			enos_restful_api_free_deleteProduct_data(dproductdata);
			product_step = 0;
			break;
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
		case CMD_LISTCERT:
			
			enos_restful_api_analysis_listCertificates(out,&listdata);
			enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",listdata->head.status);
			enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",listdata->head.requestId);
			enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",listdata->head.msg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",listdata->head.submsg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"certSN = %s\n",listdata->certData->certSN);
			enos_printf(NULL,ENOS_LOG_NOTHING,"certStatus = %d\n",listdata->certData->certStatus);
			enos_restful_api_free_listCertificates_data(listdata);
			cert_step++;
			break;
		case CMD_DELETECERT:
			cert_step++;
			enos_restful_api_analysis_revokeCertificate(out,&revdata);	
			enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",revdata->head.status);
			enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",revdata->head.requestId);
			enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",revdata->head.msg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",revdata->head.submsg);
			enos_printf(NULL,ENOS_LOG_NOTHING,"data = %s\n",revdata->data);
			enos_restful_api_free_revokeCertificate_data(revdata);
			break;
		default:
			break;
	}	
	enos_restful_api_free_output(out);
}

int main()
{
	struct enos_restful_api_struct *my_enos_restful_api;
	char request_url[]="https://beta-portal-cn4.eniot.io:8081/enosapi";
	char access_key[]="635986bc-f6cb-4812552310a8-da8c-462a";
	char access_secret[]="552310a8-da8c-462a-a8ca-f60dbdd3a97a";
	char org_id[]="o15427722038191";

//init	
	enos_restful_api_init(&my_enos_restful_api);
	
	enos_restful_api_set_requestURL(my_enos_restful_api,request_url);
	
	enos_restful_api_set_accessKey(my_enos_restful_api,access_key);
	
	enos_restful_api_set_secretKey(my_enos_restful_api,access_secret);
	
	enos_restful_api_set_orgId(my_enos_restful_api,org_id);
	
//synchronous request test
	Test_getThingModel_syn(my_enos_restful_api);
	Test_product_syn(my_enos_restful_api);
	Test_Certificate_syn(my_enos_restful_api);

//asynchronous request test	
	Test_getThingModel_asyn(my_enos_restful_api);
	while(1)
	{
		if(product_step==PWAIT_CREAT)
		{
			Test_product_asyn_creat(my_enos_restful_api);
			product_step++;
			break;
		}
	}
	
	while(1)
	{
		if(product_step==PWAIT_GET)
		{
			Test_product_asyn_get(my_enos_restful_api);
			product_step++;
			break;
		}
	}
	
	while(1)
	{
		if(product_step==PWAIT_UPDATE)
		{
			Test_product_asyn_update(my_enos_restful_api);
			product_step++;
			break;
		}
	}
	
	while(1)
	{
		if(product_step==PWAIT_DELETE)
		{
			Test_product_asyn_delete(my_enos_restful_api);
			product_step++;
			break;
		}
	}
	
	while(1)
	{
		if(cert_step==CWAIT_APPLY)
		{
			Test_Certificate_asyn_apply(my_enos_restful_api);
			cert_step++;
			break;
		}
	}
	
	while(1)
	{
		if(cert_step==CWAIT_LIST)
		{
			Test_Certificate_asyn_list(my_enos_restful_api);
			cert_step++;
			break;
		}
	}
	
	while(1)
	{
		if(cert_step==CWAIT_DELETE)
		{
			Test_Certificate_asyn_delete(my_enos_restful_api);
			cert_step++;
			break;
		}
	}
	
	sleep(5);
	return 0;
}

int Test_getThingModel_syn(struct enos_restful_api_struct *enos_restful_api)
{
	struct enos_getThingModel_input  input;
	struct enos_restful_api_output *output;
	struct enos_getThingModel_data *data;
	
	sprintf(input.thingModelId,"zhangyang_dev_model_id0");	
	enos_restful_api_syn_getThingModel(enos_restful_api,&input,&output,5);
	enos_printf(NULL,ENOS_LOG_NOTHING,"output = %s\n",output->data);
	enos_restful_api_analysis_getThingModel(output,&data);
	enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",data->head.status);
	enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",data->head.requestId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",data->head.msg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",data->head.submsg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"body = %s\n",data->head.body);
	enos_printf(NULL,ENOS_LOG_NOTHING,"data = %s\n",data->model_data);
	enos_restful_api_free_output(output);
	enos_restful_api_free_getThingModel_data(data);
		
	return 0;
}

int Test_getThingModel_asyn(struct enos_restful_api_struct *enos_restful_api)
{
	struct enos_getThingModel_input  input;
	
	sprintf(input.thingModelId,"zhangyang_dev_model_id0");		
	enos_restful_api_asyn_getThingModel(enos_restful_api,&input,restful_api_callback,NULL,CMD_GET_THING_MODEL,5);
	
	return 0;
}

int Test_product_syn(struct enos_restful_api_struct *enos_restful_api)
{
	
	struct enos_createProduct_input  input;
	struct enos_getProduct_input  ginput;
	struct enos_updateProduct_input  uinput;
	struct enos_deleteProduct_input  dinput;
	struct enos_restful_api_output *output;
	struct enos_Product_data *data;
	struct enos_deleteProduct_data *ddata;
	char productKey[ENOS_GENERAL_BUF_MAX];
	memset(productKey,0,ENOS_GENERAL_BUF_MAX);
	
	//createProduct
	sprintf(input.productName,"wangrenbin_product_test0");
	sprintf(input.productDesc,"wangrenbin_product_name0");
	sprintf(input.modelId,"zhangyang_dev_model_id0");
	input.dataType = 1;
	input.nodeType = 0;
	input.authType = 0;
	enos_restful_api_syn_createProduct(enos_restful_api,&input,&output,5);
	enos_printf(NULL,ENOS_LOG_NOTHING,"output = %s\n",output->data);
	enos_restful_api_analysis_Product(output,&data);
	
	enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",data->head.status);
	enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",data->head.requestId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",data->head.msg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",data->head.submsg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"body = %s\n",data->head.body);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productId = %s\n",data->productId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productKey = %s\n",data->productKey);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productName = %s\n",data->productName);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productSecret = %s\n",data->productSecret);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productDesc = %s\n",data->productDesc);
	enos_printf(NULL,ENOS_LOG_NOTHING,"dataType = %d\n",data->dataType);
	enos_printf(NULL,ENOS_LOG_NOTHING,"nodeType = %d\n",data->nodeType);
	enos_printf(NULL,ENOS_LOG_NOTHING,"authType = %d\n",data->authType);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productTags = %s\n",data->productTags);
	enos_printf(NULL,ENOS_LOG_NOTHING,"modelId = %s\n",data->modelId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"dynamic = %d\n",data->dynamic);
	
	sprintf(productKey,data->productKey);
	
	enos_restful_api_free_output(output);
	enos_restful_api_free_Product_data(data);
	
	//getProduct
	sprintf(ginput.productKey,productKey);
	enos_restful_api_syn_getProduct(enos_restful_api,&ginput,&output,5);
	
	enos_printf(NULL,ENOS_LOG_NOTHING,"output = %s\n",output->data);
	enos_restful_api_analysis_Product(output,&data);
	
	enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",data->head.status);
	enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",data->head.requestId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",data->head.msg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",data->head.submsg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"body = %s\n",data->head.body);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productId = %s\n",data->productId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productKey = %s\n",data->productKey);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productName = %s\n",data->productName);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productSecret = %s\n",data->productSecret);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productDesc = %s\n",data->productDesc);
	enos_printf(NULL,ENOS_LOG_NOTHING,"dataType = %d\n",data->dataType);
	enos_printf(NULL,ENOS_LOG_NOTHING,"nodeType = %d\n",data->nodeType);
	enos_printf(NULL,ENOS_LOG_NOTHING,"authType = %d\n",data->authType);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productTags = %s\n",data->productTags);
	enos_printf(NULL,ENOS_LOG_NOTHING,"modelId = %s\n",data->modelId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"dynamic = %d\n",data->dynamic);
	
	enos_restful_api_free_output(output);
	enos_restful_api_free_Product_data(data);
	
	//updateProduct
	sprintf(uinput.productKey,productKey);
	sprintf(uinput.productName,"wangrenbin_product_test1");
	sprintf(uinput.productDesc,"wangrenbin_product_name1");
	uinput.dynamic = 0;
	enos_restful_api_syn_updateProduct(enos_restful_api,&uinput,&output,5);
	
	enos_printf(NULL,ENOS_LOG_NOTHING,"output = %s\n",output->data);
	enos_restful_api_analysis_Product(output,&data);
	
	enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",data->head.status);
	enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",data->head.requestId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",data->head.msg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",data->head.submsg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"body = %s\n",data->head.body);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productId = %s\n",data->productId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productKey = %s\n",data->productKey);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productName = %s\n",data->productName);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productSecret = %s\n",data->productSecret);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productDesc = %s\n",data->productDesc);
	enos_printf(NULL,ENOS_LOG_NOTHING,"dataType = %d\n",data->dataType);
	enos_printf(NULL,ENOS_LOG_NOTHING,"nodeType = %d\n",data->nodeType);
	enos_printf(NULL,ENOS_LOG_NOTHING,"authType = %d\n",data->authType);
	enos_printf(NULL,ENOS_LOG_NOTHING,"productTags = %s\n",data->productTags);
	enos_printf(NULL,ENOS_LOG_NOTHING,"modelId = %s\n",data->modelId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"dynamic = %d\n",data->dynamic);
	
	enos_restful_api_free_output(output);
	enos_restful_api_free_Product_data(data);
	
	//deleteProduct
	sprintf(dinput.productKey,productKey);
	enos_restful_api_syn_deleteProduct(enos_restful_api,&dinput,&output,5);
	
	enos_printf(NULL,ENOS_LOG_NOTHING,"output = %s\n",output->data);
	enos_restful_api_analysis_deleteProduct(output,&ddata);
	
	enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",ddata->head.status);
	enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",ddata->head.requestId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",ddata->head.msg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",ddata->head.submsg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"body = %s\n",ddata->head.body);
	enos_printf(NULL,ENOS_LOG_NOTHING,"data = %s\n",ddata->data);
	
	enos_restful_api_free_output(output);
	enos_restful_api_free_deleteProduct_data(ddata);
	
	return 0;
}

int Test_product_asyn_creat(struct enos_restful_api_struct *enos_restful_api)
{
	struct enos_createProduct_input  input;

	memset(asyn_productKey,0,ENOS_GENERAL_BUF_MAX);
	sprintf(input.productName,"wangrenbin_product_test0");
	sprintf(input.productDesc,"wangrenbin_product_name0");
	sprintf(input.modelId,"zhangyang_dev_model_id0");
	input.dataType = 1;
	input.nodeType = 0;
	input.authType = 0;
	enos_restful_api_asyn_createProduct(enos_restful_api,&input,restful_api_callback,NULL,CMD_CREAT_PRODUCT,5);
	return 0;
}

int Test_product_asyn_get(struct enos_restful_api_struct *enos_restful_api)
{
	struct enos_getProduct_input  ginput;
	
	sprintf(ginput.productKey,asyn_productKey);
	enos_restful_api_asyn_getProduct(enos_restful_api,&ginput,restful_api_callback,NULL,CMD_GET_PRODUCT,5);
	return 0;
}

int Test_product_asyn_update(struct enos_restful_api_struct *enos_restful_api)
{
	struct enos_updateProduct_input  uinput;

	sprintf(uinput.productKey,asyn_productKey);
	sprintf(uinput.productName,"wangrenbin_product_test1");
	sprintf(uinput.productDesc,"wangrenbin_product_name1");
	uinput.dynamic = 0;
	enos_restful_api_asyn_updateProduct(enos_restful_api,&uinput,restful_api_callback,NULL,CMD_UPDATE_PRODUCT,5);
	return 0;
}

int Test_product_asyn_delete(struct enos_restful_api_struct *enos_restful_api)
{
	struct enos_deleteProduct_input  dinput;

	sprintf(dinput.productKey,asyn_productKey);
	enos_restful_api_asyn_deleteProduct(enos_restful_api,&dinput,restful_api_callback,NULL,CMD_DELETE_PRODUCT,5);
	return 0;
}

int Test_Certificate_syn(struct enos_restful_api_struct *enos_restful_api)
{
	char csr[]="-----BEGIN CERTIFICATE REQUEST-----\\nMIICtzCCAZ8CAQAwcjELMAkGA1UEBhMCQ04xETAPBgNVBAgMCFNoYW5naGFpMREw\\nDwYDVQQHDAhzaGFuZ2hhaTENMAsGA1UECgwERW5PUzESMBAGA1UECwwJRW5PUyBF\\nZGdlMRowGAYDVQQDDBExMjM0NTY3ODk4NzY1NDMyMTCCASIwDQYJKoZIhvcNAQEB\\nBQADggEPADCCAQoCggEBAMxfDZjhu24e1eneCbY8utEZQhdji5ep4L3131Uj9Www\\nWDfXT5BFkaWWj2aa37o7L05I/0w5UArhzTBBWHKzLouqp31S9M0a8V71Zo6TmssI\\nykxU8nfb+mbSMjZ2QqvOK7J0sJLORanjsyHl6A4eFLceqkQOPJdE9Pf4m2zgS59t\\nMNE1pCO+8kt+Goq6WD8ztXKO9gCQHwjgljjNxrtA+4XStZCD7+8Zu3LknaXdLBt/\\nrmXZgQnvSkeiTkp83+J0W7z6O/je5hbOPAVhCdm+P2ARw4+4/k5gHYdycJHkBtQh\\nMwhJBrv5TFIHQACAAQVqZTeiWIDf6aeQ5DVS9UmgYvMCAwEAAaAAMA0GCSqGSIb3\\nDQEBCwUAA4IBAQA1pesO5xg0h3J9uHlJ/C9ck3T+17Rsh3VEfWxdiRkbA5kJxTWU\\nDnvhTWMPgdsd2lKisYwhouMG0hX/sm2QN56kHYNHedDivr3rPT9HfFjGo8vQ9Ac/\\nV0jAdI2/56txjr6zJGrT/N8jF7XOinqKJFZMJjpP5jd1HzYvLWD6bQ+biFshbBqt\\nac5ma4PQ23Ag+oxHNOiJGIfqkwAHEQJccQNsjDmInrS5U7dP10ivYsAt7FRsQteQ\\nvG88+Vf+OjhtCfQsZe8OLwq/pFARjt6WJIXWPMki4ma5kEKxMDoXF0sKJcyjfmY1\\npnhVQd0PpGTqbIHvz8Bnp1hOp30ROJ+GCliP\\n-----END CERTIFICATE REQUEST-----";
	struct enos_applyCertificateByDeviceKey_input  input;
	struct enos_listCertificatesByDeviceKey_input  listinput;
	struct enos_revokeCertificateByDeviceKey_input  revinput;
	struct enos_restful_api_output *output;
	struct enos_applyCertificate_data *data;
	struct enos_listCertificates_data *listdata;
	struct enos_revokeCertificate_data *revdata;
	FILE *fp;
	char certSN[100];
	int i;
	
	//apply cert
	memset(certSN,0,100);
	sprintf(input.productKey,"BP3X1BTv");
	sprintf(input.deviceKey,"LSbyVxjFX9");
	input.csr = csr;
	enos_restful_api_syn_applyCertificateByDeviceKey(enos_restful_api,&input,&output,5);
	enos_printf(NULL,ENOS_LOG_NOTHING,"output = %s\n",output->data);
	
	enos_restful_api_analysis_applyCertificate(output,&data);
	enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",data->head.status);
	enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",data->head.requestId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",data->head.msg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",data->head.submsg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"certChainURL = %s\n",data->certChainURL);
	enos_printf(NULL,ENOS_LOG_NOTHING,"cert = %s\n",data->cert);
	enos_printf(NULL,ENOS_LOG_NOTHING,"certSN = %s\n",data->certSN);
	//download CA cert
	fp = fopen("D:\\program\\home\\CA.cert","a+");
	enos_restful_api_syn_downloadFile(data->certChainURL,fp,10);
	fclose(fp);
	enos_restful_api_free_applyCertificate_data(data);
	enos_restful_api_free_output(output);		
	//listCertificatesByDeviceKey
	sprintf(listinput.productKey,"BP3X1BTv");
	sprintf(listinput.deviceKey,"LSbyVxjFX9");
	enos_restful_api_syn_listCertificatesByDeviceKey(enos_restful_api,&listinput,&output,5);
	enos_printf(NULL,ENOS_LOG_NOTHING,"output = %s\n",output->data);
	
	enos_restful_api_analysis_listCertificates(output,&listdata);
	enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",listdata->head.status);
	enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",listdata->head.requestId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",listdata->head.msg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",listdata->head.submsg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"cert_num = %d\n",listdata->cert_num);
	sprintf(certSN,listdata->certData->certSN);
	for(i=0;i<listdata->cert_num;i++)
	{
		enos_printf(NULL,ENOS_LOG_NOTHING,"certSN = %s\n",listdata->certData[i].certSN);
		enos_printf(NULL,ENOS_LOG_NOTHING,"certStatus = %d\n",listdata->certData[i].certStatus);
	}
	enos_restful_api_free_listCertificates_data(listdata);
	enos_restful_api_free_output(output);		
	
	//revokeCertificateByDeviceKey
	sprintf(revinput.productKey,"BP3X1BTv");
	sprintf(revinput.deviceKey,"LSbyVxjFX9");
	sprintf(revinput.certSN,certSN);
	enos_restful_api_syn_revokeCertificateByDeviceKey(enos_restful_api,&revinput,&output,5);
	enos_printf(NULL,ENOS_LOG_NOTHING,"output = %s\n",output->data);
	enos_restful_api_analysis_revokeCertificate(output,&revdata);
	
	enos_printf(NULL,ENOS_LOG_NOTHING,"status = %d\n",revdata->head.status);
	enos_printf(NULL,ENOS_LOG_NOTHING,"requestId = %s\n",revdata->head.requestId);
	enos_printf(NULL,ENOS_LOG_NOTHING,"msg = %s\n",revdata->head.msg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"submsg = %s\n",revdata->head.submsg);
	enos_printf(NULL,ENOS_LOG_NOTHING,"data = %s\n",revdata->data);
	enos_restful_api_free_revokeCertificate_data(revdata);
	enos_restful_api_free_output(output);		
	return 0;
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

int Test_Certificate_asyn_list(struct enos_restful_api_struct *enos_restful_api)
{
	struct enos_listCertificatesByDeviceKey_input  listinput;

	sprintf(listinput.productKey,"BP3X1BTv");
	sprintf(listinput.deviceKey,"LSbyVxjFX9");
	enos_restful_api_asyn_listCertificatesByDeviceKey(enos_restful_api,&listinput,restful_api_callback, NULL, CMD_LISTCERT, 5);
	return 0;
}

int Test_Certificate_asyn_delete(struct enos_restful_api_struct *enos_restful_api)
{
	struct enos_revokeCertificateByDeviceKey_input  revinput;
	
	sprintf(revinput.productKey,"BP3X1BTv");
	sprintf(revinput.deviceKey,"LSbyVxjFX9");
	sprintf(revinput.certSN,"172");
	enos_restful_api_asyn_revokeCertificateByDeviceKey(enos_restful_api,&revinput,restful_api_callback, NULL, CMD_DELETECERT, 5);
	return 0;
}
 