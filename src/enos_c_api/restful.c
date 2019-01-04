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

#include "enos_c_api/enos_encrypt_api.h"
#include "enos_c_api/enos_restful_api.h"
#include "common_tool.h"

//compare struct enos_para_value
static int enos_para_value_compare_fun(void *para1, void *para2)
{
    char *tmp1 = (char *)(((struct enos_para_value *)para1)->para_name);
    char *tmp2 = (char *)(((struct enos_para_value *)para2)->para_name);
    return (strcmp(tmp1, tmp2));
}

static void longlongToChar(long long num,char *str)
{
	#if defined(_WIN32) || defined(WIN32)
       sprintf(str,"%I64d",num);
  #else
       sprintf(str,"%lld",num);
  #endif
}
//get time
static long long enos_getNowtime_by_ms()
{
	 struct timeval tv_now;
	 long long msec_now;
	 
	 enos_gettimeofday(&tv_now, NULL);
 	 msec_now = ((long long)tv_now.tv_sec)*1000+tv_now.tv_usec/1000; //当前时间ms
 	 
 	 return msec_now;	 
}

//url request callBack function
static size_t enos_url_Callback( void *buffer, size_t size, size_t nmemb, struct enos_restful_api_output *output ) 
{
	int segsize = size * nmemb;
	int pre_len=0;
	
	enos_printf(NULL, ENOS_LOG_ERROR,"[enos_url_Callback]size=%d,nmemb=%d\n",size,nmemb);
	
	if(segsize == 0)
		return -1 ;
	if(output == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	pre_len = output->len;
	
	output->data = (char *)realloc(output->data,pre_len+segsize+1);
	output->len += segsize;
	
	if(output->data == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	memset(&output->data[pre_len],0,segsize+1);
	memcpy(&output->data[pre_len],buffer,segsize);	
	return segsize;	
}

//url download file callback function
static size_t enos_url_getfile_Callback( void *buffer, size_t size, size_t nmemb, FILE *fp) 
{
	int segsize = size * nmemb;
	
	enos_printf(NULL, ENOS_LOG_ERROR,"file len=%d\n",segsize);
	if(fp == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
	fwrite(buffer,1,segsize,fp);
	
	return segsize;	
}
//libcurl get synchronously
static int enos_url_getMethod_syn(struct enos_restful_api_output **output,char *request_url,int timeout)
{
	CURL *curl; 
  CURLcode ret;
  struct curl_slist *headers = NULL;
	
	if((request_url == NULL)|| (output == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
  (*output) = (struct enos_restful_api_output *)malloc(sizeof(struct enos_restful_api_output));
	if((*output) == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	(*output)->len = 0;
	(*output)->url_code = CURL_LAST;
	(*output)->data = NULL;
	
  enos_printf(NULL,ENOS_LOG_NOTHING,"res_rul=%s\n",request_url); 

  curl = curl_easy_init();
  if (!curl) 
  { 
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]couldn't init curl(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    return -1; 
  }
  headers = curl_slist_append(headers, "Content-type:application/json;charset=UTF-8");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, request_url ); 
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
  
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); 
	
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)(*output));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, enos_url_Callback );
  ret = curl_easy_perform( curl );
  (*output)->url_code = ret;
  
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return 0;
}

//libcurl post synchronously
static int enos_url_postMethod_syn(struct enos_restful_api_output **output,char *request_url,char *body,int timeout)
{
	CURL *curl; 
  CURLcode ret;
  struct curl_slist *headers = NULL;
  
  if((request_url == NULL)|| (output == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
  (*output) = (struct enos_restful_api_output *)malloc(sizeof(struct enos_restful_api_output));
	if((*output) == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	(*output)->len = 0;
	(*output)->url_code = CURL_LAST;
	(*output)->data = NULL;
	
  enos_printf(NULL,ENOS_LOG_NOTHING,"res_rul=%s\n",request_url); 
  if(body!=NULL)
  	enos_printf(NULL,ENOS_LOG_NOTHING,"len=%d，body=%s\n",strlen(body),body);

  curl = curl_easy_init();
  if (!curl) 
  { 
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]couldn't init curl(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    return -1; 
  }

  headers = curl_slist_append(headers, "Content-type:application/json;charset=UTF-8");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, request_url ); 
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
  
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); 
	
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	if(body!=NULL)
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);	
	
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)(*output));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, enos_url_Callback );
  ret = curl_easy_perform( curl );
  (*output)->url_code = ret;
    
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return 0;
}

//libcurl delete synchronously
static int enos_url_deleteMethod_syn(struct enos_restful_api_output **output,char *request_url,int timeout)
{
	CURL *curl; 
  CURLcode ret;
  struct curl_slist *headers = NULL;

	if((request_url == NULL)|| (output == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
  (*output) = (struct enos_restful_api_output *)malloc(sizeof(struct enos_restful_api_output));
	if((*output) == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	(*output)->len = 0;
	(*output)->url_code = CURL_LAST;
	(*output)->data = NULL;
	
  enos_printf(NULL,ENOS_LOG_NOTHING,"res_rul=%s\n",request_url); 

  curl = curl_easy_init();
  if (!curl) 
  { 
    enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]couldn't init curl(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    return -1; 
  }
  headers = curl_slist_append(headers, "Content-type:application/json;charset=UTF-8");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, request_url ); 
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
  
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); 
	
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE"); 
	
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)(*output));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, enos_url_Callback );
  ret = curl_easy_perform( curl );

  (*output)->url_code = ret;
  
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return 0;
}

//libcurl put synchronously
static int enos_url_putMethod_syn(struct enos_restful_api_output **output,char *request_url,char *body,int timeout)
{
	CURL *curl; 
  CURLcode ret;
  struct curl_slist *headers = NULL;
  
  if((request_url == NULL) || (output == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
  (*output) = (struct enos_restful_api_output *)malloc(sizeof(struct enos_restful_api_output));
	if((*output) == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	(*output)->len = 0;
	(*output)->url_code = CURL_LAST;
	(*output)->data = NULL;
	
  enos_printf(NULL,ENOS_LOG_NOTHING,"res_rul=%s\n",request_url); 
  if(body!=NULL)
  	enos_printf(NULL,ENOS_LOG_NOTHING,"body=%s\n",body); 

  curl = curl_easy_init();
  if (!curl) 
  { 
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]couldn't init curl(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    return -1; 
  }
  headers = curl_slist_append(headers, "Content-type:application/json;charset=UTF-8");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, request_url ); 
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
  
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); 
		
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
	if(body!=NULL)
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
	
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)(*output));
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, enos_url_Callback );
  ret = curl_easy_perform( curl );
  
  (*output)->url_code = ret;
  
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  return 0;
}

//libcurl download file synchronously
static void enos_url_dowmloadFile_asyn(struct enos_url_asyn_dowmloadFile_para_struct *para)
{
	CURL *curl; 
  struct curl_slist *headers = NULL;
  
  pthread_detach(pthread_self());
  
  if(para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return;
	}
  
  enos_printf(NULL,ENOS_LOG_NOTHING,"res_rul=%s\n",para->request_url); 
  curl = curl_easy_init();
  if (!curl) 
  { 
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]couldn't init curl(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    free(para);
    para = NULL;
    return ; 
  }
  headers = curl_slist_append(headers, "Content-type:multipart/form-data;charset=UTF-8");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, para->request_url ); 
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, para->timeout);
  
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); 
	
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)para->fp);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, enos_url_getfile_Callback );
  curl_easy_perform( curl );
  
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  
  para->cb(para->user,para->fp,para->cmdID);
  free(para);
  para = NULL;
  return ;
}

//libcurl get asynchronously
static void enos_url_getMethod_asyn(struct enos_url_asyn_para_struct *para)
{
	CURL *curl; 
  CURLcode ret;
  struct curl_slist *headers = NULL;
  struct enos_restful_api_output *output;
  
  pthread_detach(pthread_self());
  
  if(para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return;
	}
	
  output = (struct enos_restful_api_output *)malloc(sizeof(struct enos_restful_api_output));
	if(output == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		free(para);
		para = NULL;
		return ;
	}
	output->len = 0;
	output->url_code = CURL_LAST;
	output->data = NULL;
  
  enos_printf(NULL,ENOS_LOG_NOTHING,"res_rul=%s\n",para->request_url); 
  curl = curl_easy_init();
  if (!curl) 
  { 
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]couldn't init curl(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    free(para);
    para = NULL;
    return ; 
  }
  headers = curl_slist_append(headers, "Content-type:application/json;charset=UTF-8");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, para->request_url ); 
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, para->timeout);
  
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); 
	
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)output);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, enos_url_Callback );
  ret = curl_easy_perform( curl );
  output->url_code = ret;
  
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  
  para->cb(para->user,output,para->cmdID);
  free(para);
  para = NULL;
  return ;
}

//libcurl post asynchronously
static void enos_url_postMethod_asyn(struct enos_url_asyn_para_struct *para)
{
	CURL *curl; 
  CURLcode ret;
  struct curl_slist *headers = NULL;
  struct enos_restful_api_output *output;
  
  pthread_detach(pthread_self());
  
  if(para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return;
	}
	
  output = (struct enos_restful_api_output *)malloc(sizeof(struct enos_restful_api_output));
	if(output == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		if(para->body!=NULL)
			free(para->body);
		free(para);
		para = NULL;
		return ;
	}
	output->len = 0;
	output->url_code = CURL_LAST;
	output->data = NULL;
  
  enos_printf(NULL,ENOS_LOG_NOTHING,"res_rul=%s\n",para->request_url); 
  curl = curl_easy_init();
  if (!curl) 
  { 
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]couldn't init curl(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    if(para->body!=NULL)
			free(para->body);
    free(para);
    para = NULL;
    return ; 
  }
  headers = curl_slist_append(headers, "Content-type:application/json;charset=UTF-8");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, para->request_url ); 
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, para->timeout);
  
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	if(para->body!=NULL)
	{
		enos_printf(NULL,ENOS_LOG_NOTHING,"body=%s\n",para->body); 
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, para->body);	
	} 
	
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)output);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, enos_url_Callback );
  ret = curl_easy_perform( curl );
  output->url_code = ret;
  
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  
  para->cb(para->user,output,para->cmdID);
  if(para->body!=NULL)
			free(para->body);
  free(para);
  para = NULL;
  return ;
}

//libcurl delete asynchronously
static void enos_url_deleteMethod_asyn(struct enos_url_asyn_para_struct *para)
{
	CURL *curl; 
  CURLcode ret;
  struct curl_slist *headers = NULL;
  struct enos_restful_api_output *output;
  
  pthread_detach(pthread_self());
  
  if(para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return;
	}
	
  output = (struct enos_restful_api_output *)malloc(sizeof(struct enos_restful_api_output));
	if(output == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		free(para);
		para = NULL;
		return ;
	}
	output->len = 0;
	output->url_code = CURL_LAST;
	output->data = NULL;
  
  enos_printf(NULL,ENOS_LOG_NOTHING,"res_rul=%s\n",para->request_url); 
  curl = curl_easy_init();
  if (!curl) 
  { 
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]couldn't init curl(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    free(para);
    para = NULL;
    return ; 
  }
  headers = curl_slist_append(headers, "Content-type:application/json;charset=UTF-8");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, para->request_url ); 
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, para->timeout);
  
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE"); 	 
	
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)output);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, enos_url_Callback );
  ret = curl_easy_perform( curl );
  output->url_code = ret;
  
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  
  para->cb(para->user,output,para->cmdID);
  free(para);
  para = NULL;
  return ;
}

//libcurl put asynchronously
static void enos_url_putMethod_asyn(struct enos_url_asyn_para_struct *para)
{
	CURL *curl; 
  CURLcode ret;
  struct curl_slist *headers = NULL;
  struct enos_restful_api_output *output;
  
  pthread_detach(pthread_self());
  
  if(para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return;
	}
	
  output = (struct enos_restful_api_output *)malloc(sizeof(struct enos_restful_api_output));
	if(output == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		free(para);
		para = NULL;
		return ;
	}
	output->len = 0;
	output->url_code = CURL_LAST;
	output->data = NULL;
  
  enos_printf(NULL,ENOS_LOG_NOTHING,"res_rul=%s\n",para->request_url); 
  curl = curl_easy_init();
  if (!curl) 
  { 
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]couldn't init curl(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    if(para->body!=NULL)
			free(para->body);
    free(para);
    para = NULL;
    return ; 
  }
  headers = curl_slist_append(headers, "Content-type:application/json;charset=UTF-8");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, para->request_url ); 
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, para->timeout);
  
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
	if(para->body!=NULL)
	{
		enos_printf(NULL,ENOS_LOG_NOTHING,"body=%s\n",para->body); 
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, para->body);
	}
	
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)output);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, enos_url_Callback );
  ret = curl_easy_perform( curl );
  output->url_code = ret;
  
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  
  para->cb(para->user,output,para->cmdID);
  if(para->body!=NULL)
			free(para->body);
  free(para);
  para = NULL;
  return ;
}
// get head data form  buffer returned by url request 
static int getHeadfromOutput(struct enos_restful_api_output_head *head,cJSON *root)
{
	cJSON *item = NULL;
	
	if((head == NULL) || (root == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]output is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	item = cJSON_GetObjectItem(root, "status");
	if(item)
	{
		head->status = item->valueint;
		item = NULL;
	}
	
	item = cJSON_GetObjectItem(root, "requestId");
	if(item)
	{
		if(item->valuestring)
		{
			head->requestId = (char *)malloc(strlen(item->valuestring)+1);
			if(head->requestId == NULL)
  		{
    	 	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 	return -1;
  		}
			memset(head->requestId,0,strlen(item->valuestring)+1);
			memcpy(head->requestId, item->valuestring, strlen(item->valuestring));
		}
		item = NULL;
	}
	item = cJSON_GetObjectItem(root, "msg");
	if(item)
	{
		if(item->valuestring)
		{
			head->msg = (char *)malloc(strlen(item->valuestring)+1);
			if(head->msg == NULL)
  		{
    	 	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 	return -1;
  		}
			memset(head->msg,0,strlen(item->valuestring)+1);
			memcpy(head->msg, item->valuestring, strlen(item->valuestring));
		}
		item = NULL;
	}
	
	item = cJSON_GetObjectItem(root, "submsg");
	if(item)
	{
		if(item->valuestring)
		{
			head->submsg = (char *)malloc(strlen(item->valuestring)+1);
			if(head->submsg == NULL)
  		{
    	 	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 	return -1;
  		}
			memset(head->submsg,0,strlen(item->valuestring)+1);
			memcpy(head->submsg, item->valuestring, strlen(item->valuestring));
		}
		item = NULL;
	}
	
	item = cJSON_GetObjectItem(root, "body");
	if(item)	
	{
		if(item->valuestring)
		{
			head->body = (char *)malloc(strlen(item->valuestring)+1);
			if(head->body == NULL)
  		{
    	 	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 	return -1;
  		}
			memset(head->body,0,strlen(item->valuestring)+1);
			memcpy(head->body, item->valuestring, strlen(item->valuestring));
		}
		item = NULL;
	}
	
	return 0;
}

static void freeDataHead(struct enos_restful_api_output_head *head)
{
	if(head != NULL)
	{
		if(head->requestId != NULL)
		{
			free(head->requestId);
			head->requestId = NULL;
		}
		
		if(head->msg != NULL)
		{
			free(head->msg);
			head->msg = NULL;
		}
		
		if(head->submsg != NULL)
		{
			free(head->submsg);
			head->submsg = NULL;
		}
		
		if(head->body != NULL)
		{
			free(head->body);
			head->body = NULL;
		}
	}
}


extern int enos_restful_api_init(struct enos_restful_api_struct **enos_ras_pp)
{
	if(enos_ras_pp == NULL)
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
	(*enos_ras_pp) = (struct enos_restful_api_struct *)malloc(sizeof(struct enos_restful_api_struct));
	if((*enos_ras_pp) == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset((*enos_ras_pp)->requestURL,0,ENOS_GENERAL_BUF_MAX);
	memset((*enos_ras_pp)->accessKey,0,ENOS_GENERAL_BUF_MAX);
	memset((*enos_ras_pp)->secretKey,0,ENOS_GENERAL_BUF_MAX);
	memset((*enos_ras_pp)->orgId,0,ENOS_GENERAL_BUF_MAX);
	
	curl_global_init(CURL_GLOBAL_ALL);
	curl_global_init(CURL_GLOBAL_SSL);
	#if defined(_WIN32) || defined(WIN32)
		curl_global_init(CURL_GLOBAL_WIN32);
	#endif
	
	return 0;
}

extern int enos_restful_api_uninit(struct enos_restful_api_struct *enos_ras_p)
{	
	if(enos_ras_p != NULL)
	{
		free(enos_ras_p);
		enos_ras_p = NULL;
	}
	return 0;
}

extern int enos_restful_api_set_requestURL(struct enos_restful_api_struct *enos_ras_p,char *requestURL)
{
	int len;
	
	if((enos_ras_p == NULL) || (requestURL == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
	len = strlen(requestURL);
	if(len > ENOS_GENERAL_BUF_MAX)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]length of requestURL is greater than ENOS_GENERAL_BUF_MAX(%d)(file=%s, function=%s, line=%d)\n",ENOS_GENERAL_BUF_MAX, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(enos_ras_p->requestURL,0,ENOS_GENERAL_BUF_MAX);
	memcpy(enos_ras_p->requestURL,requestURL,len);
	
	return 0;
}

extern int enos_restful_api_set_accessKey(struct enos_restful_api_struct *enos_ras_p,char *accessKey)
{
	int len;
	
	if((enos_ras_p == NULL) || (accessKey == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
	len = strlen(accessKey);
	if(len > ENOS_GENERAL_BUF_MAX)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]length of accessKey is greater than ENOS_GENERAL_BUF_MAX(%d)(file=%s, function=%s, line=%d)\n",ENOS_GENERAL_BUF_MAX, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(enos_ras_p->accessKey,0,ENOS_GENERAL_BUF_MAX);
	memcpy(enos_ras_p->accessKey,accessKey,len);
	
	return 0;
}

extern int enos_restful_api_set_secretKey(struct enos_restful_api_struct *enos_ras_p,char *secretKey)
{
	int len;
	
	if((enos_ras_p == NULL) || (secretKey == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
	len = strlen(secretKey);
	if(len > ENOS_GENERAL_BUF_MAX)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]length of secretKey is greater than ENOS_GENERAL_BUF_MAX(%d)(file=%s, function=%s, line=%d)\n",ENOS_GENERAL_BUF_MAX, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(enos_ras_p->secretKey,0,ENOS_GENERAL_BUF_MAX);
	memcpy(enos_ras_p->secretKey,secretKey,len);
	
	return 0;
}

extern int enos_restful_api_set_orgId(struct enos_restful_api_struct *enos_ras_p,char *orgId)
{
	int len;
	
	if((enos_ras_p == NULL) || (orgId == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
	len = strlen(orgId);
	if(len > ENOS_GENERAL_BUF_MAX)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]length of orgId is greater than ENOS_GENERAL_BUF_MAX(%d)(file=%s, function=%s, line=%d)\n",ENOS_GENERAL_BUF_MAX, __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(enos_ras_p->orgId,0,ENOS_GENERAL_BUF_MAX);
	memcpy(enos_ras_p->orgId,orgId,len);
	
	return 0;
}

extern int enos_restful_api_free_output(struct enos_restful_api_output *output)
{
	if(output!=NULL)
	{
		if(output->data != NULL)
		{
			free(output->data);
			output->data = NULL;		
		}
		free(output);
		output = NULL;
	}
	
	return 0;
}

extern int enos_calc_sign_restful(struct enos_para_value *para_value_p, int count, char *accessKey,char *secretKey, char *sign, int sign_max, int *sign_len)
{
    if((para_value_p == NULL) || (sign == NULL) || (sign_max <= 0) || (sign_len == NULL) || (accessKey == NULL)|| (secretKey == NULL) || (count <= 0))
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]para error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    
    struct enos_para_value *para_value_p_tmp = NULL;
    para_value_p_tmp = (struct enos_para_value *)malloc(sizeof(struct enos_para_value) * count);
    if(para_value_p_tmp == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]para_value_p_tmp malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memcpy(para_value_p_tmp, para_value_p, sizeof(struct enos_para_value) * count);
    
    int ret = 0;
    ret = ctool_general_merge_sort((void *)para_value_p_tmp, \
            (int)(sizeof(struct enos_para_value)), \
            count, \
            enos_para_value_compare_fun, \
            NULL, \
            NULL);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]ctool_general_merge_sort error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(para_value_p_tmp);
        para_value_p_tmp = NULL;
        return -1;
    }
    
    int sign_src_buf_max = sizeof(struct enos_para_value) * count + strlen(accessKey) + strlen(secretKey) + 10;
    char *sign_src_buf = (char *)malloc(sign_src_buf_max);
    if(sign_src_buf == NULL)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]sign_src_buf malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(para_value_p_tmp);
        para_value_p_tmp = NULL;
        return -1;
    }
    memset(sign_src_buf, 0, sign_src_buf_max);
    
    int ii = 0;
    char *sign_src_buf_ptr = sign_src_buf;
    int current_len = 0;
    //accessKey
    memcpy(sign_src_buf_ptr, accessKey, strlen(accessKey));
    sign_src_buf_ptr += strlen(accessKey);
    //para
    for(ii = 0; ii < count; ii++)
    {
        memcpy(sign_src_buf_ptr, para_value_p_tmp[ii].para_name, strlen(para_value_p_tmp[ii].para_name));
        sign_src_buf_ptr += strlen(para_value_p_tmp[ii].para_name);
        memcpy(sign_src_buf_ptr, para_value_p_tmp[ii].para_value, strlen(para_value_p_tmp[ii].para_value));
        sign_src_buf_ptr += strlen(para_value_p_tmp[ii].para_value);
    }
    //secretKey
    memcpy(sign_src_buf_ptr, secretKey, strlen(secretKey));
    sign_src_buf_ptr += strlen(secretKey);
    
    current_len = (int)(sign_src_buf_ptr - sign_src_buf);
    
    
    char sign1[256];
    int sign_max1 = (int)(sizeof(sign1));
    int sign_len1 = 0;
    memset(sign1, 0, sign_max1);
    ret = enos_hash(sign_src_buf, current_len, "sha1", sign1, sign_max1, &sign_len1);
    if(ret < 0)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]enos_hash error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
        free(para_value_p_tmp);
        free(sign_src_buf);
        return -1;
    }
    
    if(sign_max <= sign_len1 * 2)
    {
        enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]sign_max=%d < need_len=%d error(file=%s, function=%s, line=%d)\n", sign_max, sign_len1 * 2 + 1, __FILE__, __FUNCTION__, __LINE__);
        free(para_value_p_tmp);
        para_value_p_tmp = NULL;
        free(sign_src_buf);
        return -1;
    }
    
    char *sign_ptr = sign;
    for(ii = 0; ii < sign_len1; ii++)
    {
        snprintf(sign_ptr, sign_max - (sign_ptr - sign), "%02hhX", (unsigned char)(sign1[ii]));
        sign_ptr += 2;
    }
    
    *sign_len = strlen(sign);
        
    free(para_value_p_tmp);
    free(sign_src_buf);
    
    return 0;
}


extern int enos_restful_api_syn_getThingModel(struct enos_restful_api_struct *enos_ras_p, struct enos_getThingModel_input *input, struct enos_restful_api_output **output, int timeout)
{
	char request_url[REQUEST_URL_MAX_LEN];
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  CURLcode ret;
  char longstr[128];
  
  if((enos_ras_p == NULL) || (input == NULL) || (output == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(request_url,0,sizeof(request_url));
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  sprintf(request_url,"%s/modelService/thingModels/%s?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->thingModelId,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  ret = enos_url_getMethod_syn(output,request_url,timeout);
  
  return ret;
}

extern int enos_restful_api_asyn_getThingModel(struct enos_restful_api_struct *enos_ras_p, struct enos_getThingModel_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout)
{
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  char longstr[128];
  pthread_t pd;
  struct enos_url_asyn_para_struct *asyn_para;
  
  if((enos_ras_p == NULL) || (input == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
      
  asyn_para = (struct enos_url_asyn_para_struct *)malloc(sizeof(struct enos_url_asyn_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->body = NULL;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;  
  sprintf(asyn_para->request_url,"%s/modelService/thingModels/%s?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->thingModelId,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  pthread_create(&pd,NULL,(void *)&enos_url_getMethod_asyn,(struct enos_url_asyn_para_struct *) asyn_para);
  pthread_join(pd,NULL);  
  return 0;
}


extern int enos_restful_api_syn_getProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_getProduct_input *input, struct enos_restful_api_output **output, int timeout)
{
	char request_url[REQUEST_URL_MAX_LEN];
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  CURLcode ret;
  char longstr[128];
  
  if((enos_ras_p == NULL) || (input == NULL) || (output == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(request_url,0,sizeof(request_url));
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  sprintf(request_url,"%s/connectService/products/%s?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  ret = enos_url_getMethod_syn(output,request_url,timeout);
  
  return ret;
}

extern int enos_restful_api_asyn_getProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_getProduct_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout)
{
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  char longstr[128];
  pthread_t pd;
  struct enos_url_asyn_para_struct *asyn_para;
  
  if((enos_ras_p == NULL) || (input == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  asyn_para = (struct enos_url_asyn_para_struct *)malloc(sizeof(struct enos_url_asyn_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->body = NULL;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;
  sprintf(asyn_para->request_url,"%s/connectService/products/%s?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  pthread_create(&pd,NULL,(void *)&enos_url_getMethod_asyn,(struct enos_url_asyn_para_struct *) asyn_para);
  return 0;
}

extern int enos_restful_api_syn_createProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_createProduct_input *input, struct enos_restful_api_output **output, int timeout)
{
	char request_url[REQUEST_URL_MAX_LEN];
	char *body;
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  CURLcode ret;
  char longstr[128];
  cJSON *array_obj = NULL;
  
  if((enos_ras_p == NULL) || (input == NULL) || (output == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(request_url,0,sizeof(request_url));
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  sprintf(request_url,"%s/connectService/products?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  array_obj = cJSON_CreateObject();
  cJSON_AddItemToObject(array_obj, "productName", cJSON_CreateString(input->productName));
  cJSON_AddItemToObject(array_obj, "productDesc", cJSON_CreateString(input->productDesc));
  cJSON_AddItemToObject(array_obj, "modelId", cJSON_CreateString(input->modelId));
  cJSON_AddItemToObject(array_obj, "dataType", cJSON_CreateNumber(input->dataType));
  cJSON_AddItemToObject(array_obj, "nodeType", cJSON_CreateNumber(input->nodeType));
  cJSON_AddItemToObject(array_obj, "authType", cJSON_CreateNumber(input->authType));
  
  body = cJSON_Print(array_obj);
  
  ret = enos_url_postMethod_syn(output,request_url,body,timeout);
  
  free(body);
  cJSON_Delete(array_obj);
   
  return ret;
}

extern int enos_restful_api_asyn_createProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_createProduct_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout)
{
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  char longstr[128];
  cJSON *array_obj = NULL;
  pthread_t pd;
  struct enos_url_asyn_para_struct *asyn_para;
  char *body;
  
  if((enos_ras_p == NULL) || (input == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
      
  array_obj = cJSON_CreateObject();
  cJSON_AddItemToObject(array_obj, "productName", cJSON_CreateString(input->productName));
  cJSON_AddItemToObject(array_obj, "productDesc", cJSON_CreateString(input->productDesc));
  cJSON_AddItemToObject(array_obj, "modelId", cJSON_CreateString(input->modelId));
  cJSON_AddItemToObject(array_obj, "dataType", cJSON_CreateNumber(input->dataType));
  cJSON_AddItemToObject(array_obj, "nodeType", cJSON_CreateNumber(input->nodeType));
  cJSON_AddItemToObject(array_obj, "authType", cJSON_CreateNumber(input->authType));
  
  body = cJSON_Print(array_obj);
  
  asyn_para = (struct enos_url_asyn_para_struct *)malloc(sizeof(struct enos_url_asyn_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->body = body;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;
  sprintf(asyn_para->request_url,"%s/connectService/products?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
 
  pthread_create(&pd,NULL,(void *)&enos_url_postMethod_asyn,(struct enos_url_asyn_para_struct *) asyn_para);
  cJSON_Delete(array_obj);
  return 0;
}

extern int enos_restful_api_syn_deleteProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_deleteProduct_input *input, struct enos_restful_api_output **output, int timeout)
{
	char request_url[REQUEST_URL_MAX_LEN];
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  CURLcode ret;
  char longstr[128];
  
  if((enos_ras_p == NULL) || (input == NULL) || (output == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(request_url,0,sizeof(request_url));
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  sprintf(request_url,"%s/connectService/products/%s?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  ret = enos_url_deleteMethod_syn(output,request_url,timeout);
  
  return ret;
}

extern int enos_restful_api_asyn_deleteProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_deleteProduct_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout)
{
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  char longstr[128];
  pthread_t pd;
  struct enos_url_asyn_para_struct *asyn_para;
  
  if((enos_ras_p == NULL) || (input == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  asyn_para = (struct enos_url_asyn_para_struct *)malloc(sizeof(struct enos_url_asyn_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->body = NULL;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;
  sprintf(asyn_para->request_url,"%s/connectService/products/%s?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  pthread_create(&pd,NULL,(void *)&enos_url_deleteMethod_asyn,(struct enos_url_asyn_para_struct *) asyn_para);
  return 0;
}

extern int enos_restful_api_syn_updateProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_updateProduct_input *input, struct enos_restful_api_output **output, int timeout)
{
	char request_url[REQUEST_URL_MAX_LEN];
	char *body;
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  CURLcode ret;
  char longstr[128];
  cJSON *array_obj = NULL;
  
  if((enos_ras_p == NULL) || (input == NULL) || (output == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(request_url,0,sizeof(request_url));
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  sprintf(request_url,"%s/connectService/products/%s?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  array_obj = cJSON_CreateObject();
  cJSON_AddItemToObject(array_obj, "productName", cJSON_CreateString(input->productName));
  cJSON_AddItemToObject(array_obj, "productDesc", cJSON_CreateString(input->productDesc));
  cJSON_AddItemToObject(array_obj, "dynamic", cJSON_CreateBool(input->dynamic));
  
  body = cJSON_Print(array_obj);
  
  ret = enos_url_putMethod_syn(output,request_url,body,timeout);
  
  free(body);
  cJSON_Delete(array_obj);
   
  return ret;
}

extern int enos_restful_api_asyn_updateProduct(struct enos_restful_api_struct *enos_ras_p, struct enos_updateProduct_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout)
{
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  char longstr[128];
  cJSON *array_obj = NULL;
  pthread_t pd;
  struct enos_url_asyn_para_struct *asyn_para;
  char *body;
  
  if((enos_ras_p == NULL) || (input == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
      
  array_obj = cJSON_CreateObject();
  cJSON_AddItemToObject(array_obj, "productName", cJSON_CreateString(input->productName));
  cJSON_AddItemToObject(array_obj, "productDesc", cJSON_CreateString(input->productDesc));
  cJSON_AddItemToObject(array_obj, "dynamic", cJSON_CreateBool(input->dynamic));
  
  body = cJSON_Print(array_obj);
  
  asyn_para = (struct enos_url_asyn_para_struct *)malloc(sizeof(struct enos_url_asyn_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->body = body;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;
  sprintf(asyn_para->request_url,"%s/connectService/products/%s?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);

  pthread_create(&pd,NULL,(void *)&enos_url_putMethod_asyn,(struct enos_url_asyn_para_struct *) asyn_para);
  cJSON_Delete(array_obj);
  return 0;
}

extern int enos_restful_api_syn_applyCertificateByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_applyCertificateByAssetId_input *input, struct enos_restful_api_output **output, int timeout)
{
	char request_url[REQUEST_URL_MAX_LEN];
	char *body;
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  CURLcode ret;
  char longstr[128];
  int body_len;
  
  if((enos_ras_p == NULL) || (input == NULL) || (output == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(request_url,0,sizeof(request_url));
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  sprintf(request_url,"%s/connectService/devices/%s/certificates/apply?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->assetId,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  body_len = strlen(input->csr)+10;
  body = (char *)malloc(body_len);
  memset(body,0,body_len);
  body[0]='"';
  memcpy(&body[1],input->csr,strlen(input->csr));
  body[strlen(input->csr)+1]='"';
 
  ret = enos_url_postMethod_syn(output,request_url,body,timeout);
  
  free(body);

  return ret;
}

extern int enos_restful_api_asyn_applyCertificateByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_applyCertificateByAssetId_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout)
{
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  char longstr[128];
  pthread_t pd;
  struct enos_url_asyn_para_struct *asyn_para;
  char *body;
  int body_len;
  
  if((enos_ras_p == NULL) || (input == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
  
  body_len = strlen(input->csr)+10;
  body = (char *)malloc(body_len);
  memset(body,0,body_len);
  body[0]='"';
  memcpy(&body[1],input->csr,strlen(input->csr));
  body[strlen(input->csr)+1]='"';
  
  asyn_para = (struct enos_url_asyn_para_struct *)malloc(sizeof(struct enos_url_asyn_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->body = body;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;
  sprintf(asyn_para->request_url,"%s/connectService/devices/%s/certificates/apply?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->assetId,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
 
  pthread_create(&pd,NULL,(void *)&enos_url_postMethod_asyn,(struct enos_url_asyn_para_struct *) asyn_para);

  return 0;
}

extern int enos_restful_api_syn_applyCertificateByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_applyCertificateByDeviceKey_input *input, struct enos_restful_api_output **output, int timeout)
{
	char request_url[REQUEST_URL_MAX_LEN];
	char *body;
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  CURLcode ret;
  char longstr[128];
  int body_len;
  
  if((enos_ras_p == NULL) || (input == NULL) || (output == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(request_url,0,sizeof(request_url));
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  sprintf(request_url,"%s/connectService/products/%s/devices/%s/certificates/apply?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,input->deviceKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  body_len = strlen(input->csr)+10;
  body = (char *)malloc(body_len);
  memset(body,0,body_len);
  body[0]='"';
  memcpy(&body[1],input->csr,strlen(input->csr));
  body[strlen(input->csr)+1]='"';
 
  ret = enos_url_postMethod_syn(output,request_url,body,timeout);
  
  free(body);

  return ret;
}

extern int enos_restful_api_asyn_applyCertificateByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_applyCertificateByDeviceKey_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout)
{
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  char longstr[128];
  pthread_t pd;
  struct enos_url_asyn_para_struct *asyn_para;
  char *body;
  int body_len;
  
  if((enos_ras_p == NULL) || (input == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
  
  body_len = strlen(input->csr)+10;
  body = (char *)malloc(body_len);
  memset(body,0,body_len);
  body[0]='"';
  memcpy(&body[1],input->csr,strlen(input->csr));
  body[strlen(input->csr)+1]='"';
  
  asyn_para = (struct enos_url_asyn_para_struct *)malloc(sizeof(struct enos_url_asyn_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->body = body;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;
  sprintf(asyn_para->request_url,"%s/connectService/products/%s/devices/%s/certificates/apply?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,input->deviceKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
 
  pthread_create(&pd,NULL,(void *)&enos_url_postMethod_asyn,(struct enos_url_asyn_para_struct *) asyn_para);

  return 0;
}

extern int enos_restful_api_syn_listCertificatesByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_listCertificatesByAssetId_input *input, struct enos_restful_api_output **output, int timeout)
{
	char request_url[REQUEST_URL_MAX_LEN];
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  CURLcode ret;
  char longstr[128];
  
  if((enos_ras_p == NULL) || (input == NULL) || (output == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(request_url,0,sizeof(request_url));
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  sprintf(request_url,"%s/connectService/devices/%s/certificates/list?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->assetId,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  ret = enos_url_getMethod_syn(output,request_url,timeout);
  
  return ret;
}

extern int enos_restful_api_asyn_listCertificatesByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_listCertificatesByAssetId_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout)
{
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  char longstr[128];
  pthread_t pd;
  struct enos_url_asyn_para_struct *asyn_para;
  
  if((enos_ras_p == NULL) || (input == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  asyn_para = (struct enos_url_asyn_para_struct *)malloc(sizeof(struct enos_url_asyn_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->body = NULL;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;
  sprintf(asyn_para->request_url,"%s/connectService/devices/%s/certificates/list?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->assetId,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);

  pthread_create(&pd,NULL,(void *)&enos_url_getMethod_asyn,(struct enos_url_asyn_para_struct *) asyn_para);
  return 0;
}

extern int enos_restful_api_syn_listCertificatesByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_listCertificatesByDeviceKey_input *input, struct enos_restful_api_output **output, int timeout)
{
	char request_url[REQUEST_URL_MAX_LEN];
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  CURLcode ret;
  char longstr[128];
  
  if((enos_ras_p == NULL) || (input == NULL) || (output == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(request_url,0,sizeof(request_url));
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  sprintf(request_url,"%s/connectService/products/%s/devices/%s/certificates/list?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,input->deviceKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
  
  ret = enos_url_getMethod_syn(output,request_url,timeout);
  
  return ret;
}

extern int enos_restful_api_asyn_listCertificatesByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_listCertificatesByDeviceKey_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout)
{
	char sign[128];
	int sign_len;
	struct enos_para_value para[2];
  char longstr[128];
  pthread_t pd;
  struct enos_url_asyn_para_struct *asyn_para;
  
  if((enos_ras_p == NULL) || (input == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*2);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
    
  asyn_para = (struct enos_url_asyn_para_struct *)malloc(sizeof(struct enos_url_asyn_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->body = NULL;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;
  sprintf(asyn_para->request_url,"%s/connectService/products/%s/devices/%s/certificates/list?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,input->deviceKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);

  pthread_create(&pd,NULL,(void *)&enos_url_getMethod_asyn,(struct enos_url_asyn_para_struct *) asyn_para);
  return 0;
}

extern int enos_restful_api_syn_revokeCertificateByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_revokeCertificateByAssetId_input *input, struct enos_restful_api_output **output, int timeout)
{
	char request_url[REQUEST_URL_MAX_LEN];
	char sign[128];
	int sign_len;
	struct enos_para_value para[3];
  CURLcode ret;
  char longstr[128];
  
  if((enos_ras_p == NULL) || (input == NULL) || (output == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(request_url,0,sizeof(request_url));
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*3);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  if(strlen(input->certSN) == 0)
  	enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
  else
  {
  	sprintf(para[2].para_name,"certSN");
  	sprintf(para[2].para_value,input->certSN);
  	enos_calc_sign_restful(para,3,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
  }
  
  if(strlen(input->certSN) == 0) 
  	sprintf(request_url,"%s/connectService/devices/%s/certificates/revoke?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->assetId,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
	else
  	sprintf(request_url,"%s/connectService/devices/%s/certificates/revoke?accessKey=%s&certSN=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->assetId,enos_ras_p->accessKey,input->certSN,longstr,sign,enos_ras_p->orgId);
 
  ret = enos_url_deleteMethod_syn(output,request_url,timeout);
  
  return ret;
}

extern int enos_restful_api_asyn_revokeCertificateByAssetId(struct enos_restful_api_struct *enos_ras_p, struct enos_revokeCertificateByAssetId_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout)
{
	char sign[128];
	int sign_len;
	struct enos_para_value para[3];
  char longstr[128];
  pthread_t pd;
  struct enos_url_asyn_para_struct *asyn_para;
  
  if((enos_ras_p == NULL) || (input == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*3);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  if(strlen(input->certSN) == 0)
  	enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
  else
  {
  	sprintf(para[2].para_name,"certSN");
  	sprintf(para[2].para_value,input->certSN);
  	enos_calc_sign_restful(para,3,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
  }
    
  asyn_para = (struct enos_url_asyn_para_struct *)malloc(sizeof(struct enos_url_asyn_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->body = NULL;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;
  if(strlen(input->certSN) == 0) 
  	sprintf(asyn_para->request_url,"%s/connectService/devices/%s/certificates/revoke?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->assetId,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
	else
  	sprintf(asyn_para->request_url,"%s/connectService/devices/%s/certificates/revoke?accessKey=%s&certSN=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->assetId,enos_ras_p->accessKey,input->certSN,longstr,sign,enos_ras_p->orgId);

  pthread_create(&pd,NULL,(void *)&enos_url_deleteMethod_asyn,(struct enos_url_asyn_para_struct *) asyn_para);
  return 0;
}

extern int enos_restful_api_syn_revokeCertificateByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_revokeCertificateByDeviceKey_input *input, struct enos_restful_api_output **output, int timeout)
{
	char request_url[REQUEST_URL_MAX_LEN];
	char sign[128];
	int sign_len;
	struct enos_para_value para[3];
  CURLcode ret;
  char longstr[128];
  
  if((enos_ras_p == NULL) || (input == NULL) || (output == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(request_url,0,sizeof(request_url));
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*3);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  if(strlen(input->certSN) == 0)
  	enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
  else
  {
  	sprintf(para[2].para_name,"certSN");
  	sprintf(para[2].para_value,input->certSN);
  	enos_calc_sign_restful(para,3,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
  }
  
  if(strlen(input->certSN) == 0) 
  	sprintf(request_url,"%s/connectService/products/%s/devices/%s/certificates/revoke?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,input->deviceKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
	else
  	sprintf(request_url,"%s/connectService/products/%s/devices/%s/certificates/revoke?accessKey=%s&certSN=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,input->deviceKey,enos_ras_p->accessKey,input->certSN,longstr,sign,enos_ras_p->orgId);
 
  ret = enos_url_deleteMethod_syn(output,request_url,timeout);
  
  return ret;
}

extern int enos_restful_api_asyn_revokeCertificateByDeviceKey(struct enos_restful_api_struct *enos_ras_p, struct enos_revokeCertificateByDeviceKey_input *input, enos_restful_api_callback cb, void *user, int cmdID, int timeout)
{
	char sign[128];
	int sign_len;
	struct enos_para_value para[3];
  char longstr[128];
  pthread_t pd;
  struct enos_url_asyn_para_struct *asyn_para;
  
  if((enos_ras_p == NULL) || (input == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
  
  memset(sign,0,sizeof(sign));
  memset(para,0,sizeof(struct enos_para_value)*3);
  memset(longstr,0,sizeof(longstr));
  
  long long msec_now = enos_getNowtime_by_ms();
  longlongToChar(msec_now,longstr);
  
  sprintf(para[0].para_name,"requestTimestamp");
  sprintf(para[0].para_value,longstr);
  sprintf(para[1].para_name,"orgId");
  sprintf(para[1].para_value,enos_ras_p->orgId);
  if(strlen(input->certSN) == 0)
  	enos_calc_sign_restful(para,2,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
  else
  {
  	sprintf(para[2].para_name,"certSN");
  	sprintf(para[2].para_value,input->certSN);
  	enos_calc_sign_restful(para,3,enos_ras_p->accessKey,enos_ras_p->secretKey,sign,sizeof(sign),&sign_len);
  }
    
  asyn_para = (struct enos_url_asyn_para_struct *)malloc(sizeof(struct enos_url_asyn_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->body = NULL;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;
  if(strlen(input->certSN) == 0) 
  	sprintf(asyn_para->request_url,"%s/connectService/products/%s/devices/%s/certificates/revoke?accessKey=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,input->deviceKey,enos_ras_p->accessKey,longstr,sign,enos_ras_p->orgId);
	else
  	sprintf(asyn_para->request_url,"%s/connectService/products/%s/devices/%s/certificates/revoke?accessKey=%s&certSN=%s&requestTimestamp=%s&sign=%s&orgId=%s",enos_ras_p->requestURL,input->productKey,input->deviceKey,enos_ras_p->accessKey,input->certSN,longstr,sign,enos_ras_p->orgId);

  pthread_create(&pd,NULL,(void *)&enos_url_deleteMethod_asyn,(struct enos_url_asyn_para_struct *) asyn_para);
  return 0;
}

extern int enos_restful_api_analysis_getThingModel(struct enos_restful_api_output *output, struct enos_getThingModel_data **data)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	int data_temp_max;
	char *data_temp = NULL;
	struct enos_getThingModel_data *data_ret = NULL;
	char *mdata = NULL;
	
	if((output == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]output is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	if(output->url_code != CURLE_OK)
	{		
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]url_code of output not equal CURLE_OK,not analysis(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	data_temp_max = output->len + 1;
	data_temp = (char *)malloc(data_temp_max);  
  if(data_temp == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     return -1;
  }
  memset(data_temp, 0, data_temp_max);
  memcpy(data_temp, output->data, output->len);
	
	data_ret = (struct enos_getThingModel_data *)malloc(sizeof(struct enos_getThingModel_data));
	if(data_ret == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     free(data_temp);
     return -1;
  }
	memset(data_ret, 0, sizeof(struct enos_getThingModel_data));
		
	root = cJSON_Parse(data_temp);
	if(!root)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]cJSON_Parse err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_getThingModel_data(data_ret);
		return -1;
	}
	
	if(getHeadfromOutput((struct enos_restful_api_output_head *)&data_ret->head,root) < 0)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]getHeadfromOutput err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_getThingModel_data(data_ret);
		return -1;
	}
	
	item = cJSON_GetObjectItem(root, "data");
	if(item)	
	{
			mdata = cJSON_Print(item);
			data_ret->model_data = (char *)malloc(strlen(mdata)+1);
			if(data_ret->model_data == NULL)
  		{
    	 	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 	free(data_temp);
    	 	free(mdata);
				enos_restful_api_free_getThingModel_data(data_ret);
    	 	return -1;
  		}
			memset(data_ret->model_data,0,strlen(mdata)+1);
			memcpy(data_ret->model_data, mdata, strlen(mdata));
			free(mdata);
	}
	(*data) = data_ret;
	free(data_temp);
	cJSON_Delete(root);
	return 0;
}

extern int enos_restful_api_free_getThingModel_data(struct enos_getThingModel_data *data)
{
	if(data != NULL)
	{
		freeDataHead((struct enos_restful_api_output_head *)&data->head);	
		
		if(data->model_data != NULL)
		{
			free(data->model_data);
			data->model_data = NULL;
		}
		
		free(data);
		data = NULL;
	}
	return 0;
}

extern int enos_restful_api_analysis_Product(struct enos_restful_api_output *output, struct enos_Product_data **data)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	cJSON *sub_item = NULL;
	int data_temp_max;
	char *data_temp = NULL;
	struct enos_Product_data *data_ret = NULL;
	char *mdata = NULL;
	
	if((output == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]output is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	if(output->url_code != CURLE_OK)
	{		
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]url_code of output not equal CURLE_OK,not analysis(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	data_temp_max = output->len + 1;
	data_temp = (char *)malloc(data_temp_max);  
  if(data_temp == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     return -1;
  }
  memset(data_temp, 0, data_temp_max);
  memcpy(data_temp, output->data, output->len);
	
	data_ret = (struct enos_Product_data *)malloc(sizeof(struct enos_Product_data));
	if(data_ret == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     free(data_temp);
     return -1;
  }
	memset(data_ret, 0, sizeof(struct enos_Product_data));
		
	root = cJSON_Parse(data_temp);
	if(!root)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]cJSON_Parse err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_Product_data(data_ret);
		return -1;
	}
	
	if(getHeadfromOutput((struct enos_restful_api_output_head *)&data_ret->head,root) < 0)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]getHeadfromOutput err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_Product_data(data_ret);
		return -1;
	}
	item = cJSON_GetObjectItem(root, "data");
	if(item)	
	{
		if(cJSON_Object == item->type)
		{
			sub_item = cJSON_GetObjectItem(item,"productId");
			if(sub_item)
			{
				if(sub_item->valuestring)
				{
					data_ret->productId = (char *)malloc(strlen(sub_item->valuestring)+1);
					if(data_ret->productId == NULL)
  				{
    	 			enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 			free(data_temp);
						enos_restful_api_free_Product_data(data_ret);
    	 			return -1;
  				}
  				memset(data_ret->productId,0,strlen(sub_item->valuestring)+1);
					memcpy(data_ret->productId, sub_item->valuestring, strlen(sub_item->valuestring));
				}
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"productId");
			if(sub_item)
			{
				if(sub_item->valuestring)
				{
					data_ret->productId = (char *)malloc(strlen(sub_item->valuestring)+1);
					if(data_ret->productId == NULL)
  				{
    	 			enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 			free(data_temp);
						enos_restful_api_free_Product_data(data_ret);
    	 			return -1;
  				}
  				memset(data_ret->productId,0,strlen(sub_item->valuestring)+1);
					memcpy(data_ret->productId, sub_item->valuestring, strlen(sub_item->valuestring));
				}
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"productKey");
			if(sub_item)
			{
				if(sub_item->valuestring)
				{
					data_ret->productKey = (char *)malloc(strlen(sub_item->valuestring)+1);
					if(data_ret->productKey == NULL)
  				{
    	 			enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 			free(data_temp);
						enos_restful_api_free_Product_data(data_ret);
    	 			return -1;
  				}
  				memset(data_ret->productKey,0,strlen(sub_item->valuestring)+1);
					memcpy(data_ret->productKey, sub_item->valuestring, strlen(sub_item->valuestring));
				}
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"productName");
			if(sub_item)
			{
				if(sub_item->valuestring)
				{
					data_ret->productName = (char *)malloc(strlen(sub_item->valuestring)+1);
					if(data_ret->productName == NULL)
  				{
    	 			enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 			free(data_temp);
						enos_restful_api_free_Product_data(data_ret);
    	 			return -1;
  				}
  				memset(data_ret->productName,0,strlen(sub_item->valuestring)+1);
					memcpy(data_ret->productName, sub_item->valuestring, strlen(sub_item->valuestring));
				}
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"productSecret");
			if(sub_item)
			{
				if(sub_item->valuestring)
				{
					data_ret->productSecret = (char *)malloc(strlen(sub_item->valuestring)+1);
					if(data_ret->productSecret == NULL)
  				{
    	 			enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 			free(data_temp);
						enos_restful_api_free_Product_data(data_ret);
    	 			return -1;
  				}
  				memset(data_ret->productSecret,0,strlen(sub_item->valuestring)+1);
					memcpy(data_ret->productSecret, sub_item->valuestring, strlen(sub_item->valuestring));
				}
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"productDesc");
			if(sub_item)
			{
				if(sub_item->valuestring)
				{
					data_ret->productDesc = (char *)malloc(strlen(sub_item->valuestring)+1);
					if(data_ret->productDesc == NULL)
  				{
    	 			enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 			free(data_temp);
						enos_restful_api_free_Product_data(data_ret);
    	 			return -1;
  				}
  				memset(data_ret->productDesc,0,strlen(sub_item->valuestring)+1);
					memcpy(data_ret->productDesc, sub_item->valuestring, strlen(sub_item->valuestring));
				}
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"dataType");
			if(sub_item)
			{
				data_ret->dataType = sub_item->valueint;
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"nodeType");
			if(sub_item)
			{
				data_ret->nodeType = sub_item->valueint;
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"authType");
			if(sub_item)
			{
				data_ret->authType = sub_item->valueint;
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"productTags");
			if(sub_item)
			{
				mdata = cJSON_Print(sub_item);
				data_ret->productTags = (char *)malloc(strlen(mdata)+1);
				if(data_ret->productTags == NULL)
  			{
    	 		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 		free(data_temp);
    	 		free(mdata);
					enos_restful_api_free_Product_data(data_ret);
    	 		return -1;
  			}
  			memset(data_ret->productTags,0,strlen(mdata)+1);
				memcpy(data_ret->productTags, mdata, strlen(mdata));				
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"modelId");
			if(sub_item)
			{
				if(sub_item->valuestring)
				{
					data_ret->modelId = (char *)malloc(strlen(sub_item->valuestring)+1);
					if(data_ret->modelId == NULL)
  				{
    	 			enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 			free(data_temp);
						enos_restful_api_free_Product_data(data_ret);
    	 			return -1;
  				}
  				memset(data_ret->modelId,0,strlen(sub_item->valuestring)+1);
					memcpy(data_ret->modelId, sub_item->valuestring, strlen(sub_item->valuestring));
				}
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"dynamic");
			if(sub_item)
			{
				data_ret->dynamic = sub_item->valueint;
				sub_item = NULL;
			}
		}
	}
	(*data) = data_ret;
	free(data_temp);
	cJSON_Delete(root);
	return 0;
}

extern int enos_restful_api_free_Product_data(struct enos_Product_data *data)
{
	if(data != NULL)
	{
		freeDataHead((struct enos_restful_api_output_head *)&data->head);	
		
		if(data->productId != NULL)
		{
			free(data->productId);
			data->productId = NULL;
		}
		
		if(data->productKey != NULL)
		{
			free(data->productKey);
			data->productKey = NULL;
		}
		
		if(data->productName != NULL)
		{
			free(data->productName);
			data->productName = NULL;
		}
		
		if(data->productSecret != NULL)
		{
			free(data->productSecret);
			data->productSecret = NULL;
		}
		
		if(data->productDesc != NULL)
		{
			free(data->productDesc);
			data->productDesc = NULL;
		}
		
		if(data->productTags != NULL)
		{
			free(data->productTags);
			data->productTags = NULL;
		}
		
		if(data->modelId != NULL)
		{
			free(data->modelId);
			data->modelId = NULL;
		}
		
		free(data);
		data = NULL;
	}
	return 0;
}

extern int enos_restful_api_analysis_deleteProduct(struct enos_restful_api_output *output, struct enos_deleteProduct_data **data)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	cJSON *sub_item = NULL;
	int data_temp_max;
	char *data_temp = NULL;
	struct enos_deleteProduct_data *data_ret = NULL;
	
	if((output == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]output is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	if(output->url_code != CURLE_OK)
	{		
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]url_code of output not equal CURLE_OK,not analysis(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	data_temp_max = output->len + 1;
	data_temp = (char *)malloc(data_temp_max);  
  if(data_temp == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     return -1;
  }
  memset(data_temp, 0, data_temp_max);
  memcpy(data_temp, output->data, output->len);
	
	data_ret = (struct enos_deleteProduct_data *)malloc(sizeof(struct enos_deleteProduct_data));
	if(data_ret == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     free(data_temp);
     return -1;
  }
	memset(data_ret, 0, sizeof(struct enos_deleteProduct_data));
		
	root = cJSON_Parse(data_temp);
	if(!root)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]cJSON_Parse err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_deleteProduct_data(data_ret);
		return -1;
	}
	
	if(getHeadfromOutput((struct enos_restful_api_output_head *)&data_ret->head,root) < 0)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]getHeadfromOutput err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_deleteProduct_data(data_ret);
		return -1;
	}
	item = cJSON_GetObjectItem(root, "data");
	if(item)
	{
		if(item->valuestring)
		{
			data_ret->data = (char *)malloc(strlen(sub_item->valuestring)+1);
			if(data_ret->data == NULL)
  		{
   			enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
   			free(data_temp);
				enos_restful_api_free_deleteProduct_data(data_ret);
   			return -1;
  		}
  		memset(data_ret->data,0,strlen(sub_item->valuestring)+1);
			memcpy(data_ret->data, sub_item->valuestring, strlen(sub_item->valuestring));
		}
	}

	(*data) = data_ret;
	free(data_temp);
	cJSON_Delete(root);
	return 0;
}

extern int enos_restful_api_free_deleteProduct_data(struct enos_deleteProduct_data *data)
{
	if(data != NULL)
	{
		freeDataHead((struct enos_restful_api_output_head *)&data->head);	
		
		if(data->data != NULL)
		{
			free(data->data);
			data->data = NULL;
		}
		
		free(data);
		data = NULL;
	}
	return 0;
}

extern int enos_restful_api_analysis_applyCertificate(struct enos_restful_api_output *output, struct enos_applyCertificate_data **data)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	cJSON *sub_item=NULL;
	int data_temp_max;
	char *data_temp = NULL;
	struct enos_applyCertificate_data *data_ret = NULL;

	
	if((output == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]output is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	if(output->url_code != CURLE_OK)
	{		
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]url_code of output not equal CURLE_OK,not analysis(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	data_temp_max = output->len + 1;
	data_temp = (char *)malloc(data_temp_max);  
  if(data_temp == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     return -1;
  }
  memset(data_temp, 0, data_temp_max);
  memcpy(data_temp, output->data, output->len);
	
	data_ret = (struct enos_applyCertificate_data *)malloc(sizeof(struct enos_applyCertificate_data));
	if(data_ret == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     free(data_temp);
     return -1;
  }
	memset(data_ret, 0, sizeof(struct enos_applyCertificate_data));
		
	root = cJSON_Parse(data_temp);
	if(!root)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]cJSON_Parse err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_applyCertificate_data(data_ret);
		return -1;
	}
	
	if(getHeadfromOutput((struct enos_restful_api_output_head *)&data_ret->head,root) < 0)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]getHeadfromOutput err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_applyCertificate_data(data_ret);
		return -1;
	}
	
	item = cJSON_GetObjectItem(root, "data");
	if(item)	
	{
		if(cJSON_Object == item->type)
		{
			sub_item = cJSON_GetObjectItem(item,"certChainURL");
			if(sub_item)
			{
				if(sub_item->valuestring)
				{
					data_ret->certChainURL = (char *)malloc(strlen(sub_item->valuestring)+1);
					if(data_ret->certChainURL == NULL)
  				{
    	 			enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 			free(data_temp);
						enos_restful_api_free_applyCertificate_data(data_ret);
    	 			return -1;
  				}
  				memset(data_ret->certChainURL,0,strlen(sub_item->valuestring)+1);
					memcpy(data_ret->certChainURL, sub_item->valuestring, strlen(sub_item->valuestring));
				}
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"cert");
			if(sub_item)
			{
				if(sub_item->valuestring)
				{
					data_ret->cert = (char *)malloc(strlen(sub_item->valuestring)+1);
					if(data_ret->cert == NULL)
  				{
    	 			enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 			free(data_temp);
						enos_restful_api_free_applyCertificate_data(data_ret);
    	 			return -1;
  				}
  				memset(data_ret->cert,0,strlen(sub_item->valuestring)+1);
					memcpy(data_ret->cert, sub_item->valuestring, strlen(sub_item->valuestring));
				}
				sub_item = NULL;
			}
			
			sub_item = cJSON_GetObjectItem(item,"certSN");
			if(sub_item)
			{
				if(sub_item->valuestring)
				{
					data_ret->certSN = (char *)malloc(strlen(sub_item->valuestring)+1);
					if(data_ret->certSN == NULL)
  				{
    	 			enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 			free(data_temp);
						enos_restful_api_free_applyCertificate_data(data_ret);
    	 			return -1;
  				}
  				memset(data_ret->certSN,0,strlen(sub_item->valuestring)+1);
					memcpy(data_ret->certSN, sub_item->valuestring, strlen(sub_item->valuestring));
				}
				sub_item = NULL;
			}	
		}
	}
	(*data) = data_ret;
	free(data_temp);
	cJSON_Delete(root);
	return 0;
}

extern int enos_restful_api_free_applyCertificate_data(struct enos_applyCertificate_data *data)
{
	if(data != NULL)
	{
		freeDataHead((struct enos_restful_api_output_head *)&data->head);	
		
		if(data->certChainURL != NULL)
		{
			free(data->certChainURL);
			data->certChainURL = NULL;
		}
		
		if(data->cert != NULL)
		{
			free(data->cert);
			data->cert = NULL;
		}
		
		if(data->certSN != NULL)
		{
			free(data->certSN);
			data->certSN = NULL;
		}
		
		free(data);
		data = NULL;
	}
	return 0;
}

extern int enos_restful_api_analysis_listCertificates(struct enos_restful_api_output *output, struct enos_listCertificates_data **data)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	cJSON *sub_item=NULL;
	cJSON *sub_obj=NULL;
	int data_temp_max;
	char *data_temp = NULL;
	struct enos_listCertificates_data *data_ret = NULL;
	int array_num=0;
	int i;

	
	if((output == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]output is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	if(output->url_code != CURLE_OK)
	{		
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]url_code of output not equal CURLE_OK,not analysis(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	data_temp_max = output->len + 1;
	data_temp = (char *)malloc(data_temp_max);  
  if(data_temp == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     return -1;
  }
  memset(data_temp, 0, data_temp_max);
  memcpy(data_temp, output->data, output->len);
	
	data_ret = (struct enos_listCertificates_data *)malloc(sizeof(struct enos_listCertificates_data));
	if(data_ret == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     free(data_temp);
     return -1;
  }
	memset(data_ret, 0, sizeof(struct enos_listCertificates_data));
		
	root = cJSON_Parse(data_temp);
	if(!root)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]cJSON_Parse err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_listCertificates_data(data_ret);
		return -1;
	}
	
	if(getHeadfromOutput((struct enos_restful_api_output_head *)&data_ret->head,root) < 0)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]getHeadfromOutput err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_listCertificates_data(data_ret);
		return -1;
	}
	
	item = cJSON_GetObjectItem(root, "data");
	if(item)	
	{
		if(cJSON_Array == item->type)
		{
			array_num = cJSON_GetArraySize(item);
			if(array_num > 0)
			{
				data_ret->certData = (struct enos_listCertificates_certData *)malloc(sizeof(struct enos_listCertificates_certData)*array_num);
				if(data_ret->certData == NULL)
  			{
    	 		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 		free(data_temp);
					enos_restful_api_free_listCertificates_data(data_ret);
    	 		return -1;
  			}
  			data_ret->cert_num = array_num;
  			for(i=0;i<array_num;i++)
  			{
  				sub_obj = cJSON_GetArrayItem(item,i); 			
  				sub_item = cJSON_GetObjectItem(sub_obj, "certSN");
  				if(sub_item)
  				{
  					if(sub_item->valuestring)
  					{
  						data_ret->certData[i].certSN = (char *)malloc(strlen(sub_item->valuestring)+1);
  						if(data_ret->certData[i].certSN == NULL)
  						{
    	 					enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 					free(data_temp);
								enos_restful_api_free_listCertificates_data(data_ret);
    	 					return -1;
  						}
  						memset(data_ret->certData[i].certSN,0,strlen(sub_item->valuestring)+1);
							memcpy(data_ret->certData[i].certSN, sub_item->valuestring, strlen(sub_item->valuestring));
						}
						sub_item = NULL;
					}
					
					sub_item = cJSON_GetObjectItem(sub_obj, "certStatus");
					if(sub_item)
  				{
  					data_ret->certData[i].certStatus = sub_item->valueint;
  					sub_item = NULL;
  				}
  			}
  		}		
		}	
	}
	(*data) = data_ret;
	free(data_temp);
	cJSON_Delete(root);
	return 0;
}

extern int enos_restful_api_free_listCertificates_data(struct enos_listCertificates_data *data)
{
	int i;
	
	if(data != NULL)
	{
		freeDataHead((struct enos_restful_api_output_head *)&data->head);	
		if(data->certData != NULL)
		{
			for(i=0;i<data->cert_num;i++)
			{
				if(data->certData[i].certSN != NULL)
				{
					free(data->certData[i].certSN);
					data->certData[i].certSN = NULL;
				}
			}	
			free(data->certData);
			data->certData = NULL;
		}	
		free(data);
		data = NULL;
	}
	return 0;
}

extern int enos_restful_api_analysis_revokeCertificate(struct enos_restful_api_output *output, struct enos_revokeCertificate_data **data)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	int data_temp_max;
	char *data_temp = NULL;
	struct enos_revokeCertificate_data *data_ret = NULL;

	
	if((output == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]output is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	if(output->url_code != CURLE_OK)
	{		
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]url_code of output not equal CURLE_OK,not analysis(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);		
		return -1;
	}
	
	data_temp_max = output->len + 1;
	data_temp = (char *)malloc(data_temp_max);  
  if(data_temp == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     return -1;
  }
  memset(data_temp, 0, data_temp_max);
  memcpy(data_temp, output->data, output->len);
	
	data_ret = (struct enos_revokeCertificate_data *)malloc(sizeof(struct enos_revokeCertificate_data));
	if(data_ret == NULL)
  {
     enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
     free(data_temp);
     return -1;
  }
	memset(data_ret, 0, sizeof(struct enos_revokeCertificate_data));
		
	root = cJSON_Parse(data_temp);
	if(!root)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]cJSON_Parse err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_revokeCertificate_data(data_ret);
		return -1;
	}
	
	if(getHeadfromOutput((struct enos_restful_api_output_head *)&data_ret->head,root) < 0)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]getHeadfromOutput err(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);				
		free(data_temp);
		enos_restful_api_free_revokeCertificate_data(data_ret);
		return -1;
	}
	
	item = cJSON_GetObjectItem(root, "data");
	if(item)	
	{
		if(item->valuestring)
		{
			data_ret->data = (char *)malloc(strlen(item->valuestring)+1);
			if(data_ret->data == NULL)
  		{
    	 	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc error(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    	 	free(data_temp);
				enos_restful_api_free_revokeCertificate_data(data_ret);
    	 	return -1;
  		}
			memset(data_ret->data,0,strlen(item->valuestring)+1);
			memcpy(data_ret->data, item->valuestring, strlen(item->valuestring));	
		}
	}
	(*data) = data_ret;
	free(data_temp);
	cJSON_Delete(root);
	return 0;
}

extern int enos_restful_api_free_revokeCertificate_data(struct enos_revokeCertificate_data *data)
{
	if(data != NULL)
	{
		freeDataHead((struct enos_restful_api_output_head *)&data->head);	
		
		if(data->data != NULL)
		{			
			free(data->data);	
			data->data = NULL;
		}	
		free(data);
		data = NULL;
	}
	return 0;
}

extern int enos_restful_api_syn_downloadFile(char *request_url,FILE *fp,int timeout)
{
	CURL *curl; 
  CURLcode ret;
  struct curl_slist *headers = NULL;
  
  if((request_url == NULL) || (fp == NULL))
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]pointer is NULL(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	
  enos_printf(NULL,ENOS_LOG_NOTHING,"res_rul=%s\n",request_url); 

  curl = curl_easy_init();
  if (!curl) 
  { 
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]couldn't init curl(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
    return -1; 
  }
  headers = curl_slist_append(headers, "Content-type:multipart/form-data;charset=UTF-8");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, request_url ); 
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
  
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);//设定为不验证证书和HOST 
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0); 
	
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)fp);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, enos_url_getfile_Callback );
  ret = curl_easy_perform( curl );
  
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  
  if(ret != CURLE_OK)
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]CURL RET(%d) NOT EQUAL CURLE_OK(file=%s, function=%s, line=%d)\n",ret, __FILE__, __FUNCTION__, __LINE__);
		return -1;   	
  }
  return 0;
}

extern int enos_restful_api_asyn_downloadFile(char *request_url,enos_restful_api_dowmloadFile_callback cb,FILE *fp,void *user,int  cmdID,int timeout)
{
  pthread_t pd;
  struct enos_url_asyn_dowmloadFile_para_struct *asyn_para;
  
  if((request_url == NULL) || (fp == NULL))
  {
  	enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]point is null(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
  }
    
  asyn_para = (struct enos_url_asyn_dowmloadFile_para_struct *)malloc(sizeof(struct enos_url_asyn_dowmloadFile_para_struct));
	if(asyn_para == NULL)
	{
		enos_printf(NULL, ENOS_LOG_ERROR, "[RESTFUL_API]malloc fail(file=%s, function=%s, line=%d)\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}
	memset(asyn_para,0,sizeof(struct enos_url_asyn_para_struct));
  asyn_para->cb = cb;
  asyn_para->fp = fp;
  asyn_para->user = user;
  asyn_para->cmdID = cmdID;
  asyn_para->timeout = timeout;
  sprintf(asyn_para->request_url,request_url);
  
  pthread_create(&pd,NULL,(void *)&enos_url_dowmloadFile_asyn,(struct enos_url_asyn_dowmloadFile_para_struct *) asyn_para);
  return 0;
}






