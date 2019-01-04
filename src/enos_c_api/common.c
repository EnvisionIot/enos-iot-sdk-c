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

#include "enos_c_api/enos_common.h"

extern int enos_printf(void *print_arg, int print_level, const char *format, ...)
{
    if(print_level > ENOS_CURRENT_LOG_LEVEL)
    {
        return -1;
    }

    char sprint_buf[1024];
    va_list args;
    int n;
    va_start(args, format);
    n = vsnprintf(sprint_buf, sizeof(sprint_buf) - 1, format, args);
    va_end(args);
    
    if(n < 0)
    {
        return -1;
    }
    else if(n > (int)(sizeof(sprint_buf)))
    {
        int buf_max = n;
        char *sprint_buf_temp = (char *)malloc(buf_max);
        if(sprint_buf_temp == NULL)
        {
            return -1;
        }
        va_start(args, format);
        n = vsnprintf(sprint_buf_temp, buf_max, format, args);
        va_end(args);
        
        if((n < 0) || (n > buf_max))
        {
            free(sprint_buf_temp);
            return -1;
        }
        
        write(STDOUT_FILENO, sprint_buf_temp, n);
        free(sprint_buf_temp);
        return n;
    }
    else
    {
        write(STDOUT_FILENO, sprint_buf, n);
        return n;
    }
}

extern int enos_char_code_convert(char *from_charset, char *to_charset, char *inbuf, int inlen, char *outbuf, int outbuf_max_len, int *out_len)
{
    if((from_charset == NULL) || (to_charset == NULL) || (inbuf == NULL) || (inlen < 0) || (outbuf == NULL) || (outbuf_max_len <= 0) || (out_len == NULL))
    {
        return -1;
    }

    if(inlen == 0)
    {
        memset(outbuf, 0, outbuf_max_len);
        *out_len = 0;
        return 0;
    }
    
    if(strcmp(from_charset, to_charset) == 0)
    {
        if(inlen >= outbuf_max_len)
        {
            return -1;
        }
        else
        {
            memset(outbuf, 0, outbuf_max_len);
            memcpy(outbuf, inbuf, inlen);
            *out_len = inlen;
            return 0;
        }
    }

    iconv_t fd = iconv_open(to_charset, from_charset);
    if(fd == NULL)
    {
        return -1;
    }

    int ret = 0;
    char *pin = (char *)inbuf;

    char *outbuf_temp = NULL;
    int outbuf_temp_max = inlen * 6;
    outbuf_temp = (char *)malloc(outbuf_temp_max);
    if(outbuf_temp == NULL)
    {
        iconv_close(fd);
        return -1;
    }
    memset(outbuf_temp, 0, outbuf_temp_max);

    char *outbuf_temp_p = outbuf_temp;
    size_t inlen_size_t = (size_t)inlen;
    size_t outlen_size_t = (size_t)outbuf_temp_max;
    #if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
        //ret = iconv(fd, (const char **)&pin, &inlen_size_t, &outbuf_temp_p, &outlen_size_t);
        ret = iconv(fd, &pin, &inlen_size_t, &outbuf_temp_p, &outlen_size_t);
    #else
        ret = iconv(fd, &pin, &inlen_size_t, &outbuf_temp_p, &outlen_size_t);
    #endif
    if(ret < 0)
    {
        iconv_close(fd);
        free(outbuf_temp);
        return -1;
    }

    int dst_len = outbuf_temp_max - (int)outlen_size_t;
    if(dst_len < 0)
    {
        iconv_close(fd);
        free(outbuf_temp);
        return -1;
    }

    if(dst_len >= outbuf_max_len)
    {
        iconv_close(fd);
        free(outbuf_temp);
        return -1;
    }
    memset(outbuf, 0, outbuf_max_len);
    memcpy(outbuf, outbuf_temp, dst_len);
    *out_len = dst_len;

    iconv_close(fd);
    free(outbuf_temp);

    return 0;
}

extern int enos_gettimeofday(struct timeval *tvp, void *tzp)
{
    #if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
        struct timeb timeb_temp;
        memset(&timeb_temp, 0, sizeof(timeb_temp));
        ftime(&timeb_temp);
        tvp->tv_sec = timeb_temp.time;
        tvp->tv_usec = timeb_temp.millitm * 1000;
        return 0;
    #else
        int ret = gettimeofday(tvp, (struct timezone *)tzp);
        if(ret < 0)
        {
            return -1;
        }
        return 0;
    #endif
}

extern long long int enos_cal_time_gap(struct timeval *tvp1, struct timeval *tvp2)
{
    long long int sec = tvp2->tv_sec - tvp1->tv_sec;
    long long int usec = tvp2->tv_usec - tvp1->tv_usec;
    long long int msec = sec * 1000 + usec / 1000;
    return msec;
}

extern long long int enos_cal_time_elapsed(struct timeval *tvp)
{
    struct timeval tv_temp;
    enos_gettimeofday(&tv_temp, NULL);
    return enos_cal_time_gap(tvp, &tv_temp);
}

extern int enos_sleep(long long int ms)
{
    long ms_long = (long)ms;
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	Sleep(ms_long);
#else
	usleep(ms_long * 1000);
#endif
    return 0;
}
