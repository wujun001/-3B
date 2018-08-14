#include "support.h"
#include <iostream>
#include <regex>
#include <stdio.h>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>

using namespace std;

char *pszNian[]={"","十","百","千","万","十","百","千","亿"};
char *pszShu[]={"零","壹","贰","叁","肆","伍","陆","柒","捌","玖"};


// match 1: 22 优 (with a length of 6)
// match 2: 20 (with a length of 2)
// match 3: 阴 (with a length of 3)
// match 4: 87 (with a length of 10)
// match 5: 西北风2级 (with a length of 13)
// match 6: 冷热适宜，感觉很舒适。 (with a length of 33)
std::smatch results;

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    std::string* str = dynamic_cast<std::string*>((std::string *)lpVoid);
    if( NULL == str || NULL == buffer )
    {
        return -1;
    }

    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    //*str = UTF8ToGBK(*str);   //服务器采用UTF8，本程序采用GBK编码，需要转换一下
    return nmemb;
}

int dealResCode(CURL* curl, const CURLcode res)
{
    //输出返回码代表的意思
    int nCode = 0;
    const char* pRes = NULL;
    pRes = curl_easy_strerror(res);
    printf("%s\n",pRes);

    //http返回码
    long lResCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &lResCode);

    if(CURLE_OK != res || 200 != lResCode)
    {
        //curl传送失败
        if(CURLE_OPERATION_TIMEOUTED == res)
        {
            nCode = 1;   //超时返回
        }
        else
        {
            nCode = -1;    //其它错误返回
        }
        printf("curl send msg error: pRes=%s, lResCode=%ld", pRes, lResCode);
    }

    return nCode;
}


int Get(const std::string & strUrl, std::string & strResponse)
{
    CURLcode res;
    CURL* curl = curl_easy_init();
    if(NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }

    //std:string strUrlUTF8 = GBKToUTF8(strUrl);   //服务器采用UTF8方式
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&strResponse);
    /**
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
    */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
    res = curl_easy_perform(curl);
    dealResCode(curl, res); //处理错误码
    curl_easy_cleanup(curl);
    return res;
}




//char digitToChinese[200];

//digitToChinese
string digitToChinese(int a,int b)
{
    string d2c;

    if (0 == b){
        d2c.clear();
    }

    if(a>=10)
    {
        digitToChinese(a/10,b+1);
    }
    d2c += pszShu[a%10];
    d2c += pszNian[b];
    //sprintf(digitToChinese, "%s%s%s",digitToChinese, pszShu[a%10],pszNian[b]);
    return d2c;
}




//需要jsoncpp库
bool sojsonWeatherAPI(){
    string strOutData;
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value jsonOutData;

    string strIndata("https://www.sojson.com/open/api/weather/json.shtml?city=%E6%A0%AA%E6%B4%B2");

    int CURLCode = Get(strIndata, strOutData); //"\{\"car_num\":\"湘B12345\",\"parking_code\":\"C00002\"}"
    if(strOutData[0] == '{'){//json字串
        reader.parse(strOutData, jsonOutData);
        if (jsonOutData["status"].asString() == "200"){//成功
            //printf("成功 入参 %s, 出参%s", strIndata.c_str(), strOutData.c_str());
            string sound("mplayer \"http://tts.baidu.com/text2audio?");
            sound += "idx=1&cuid=baidu_speech_demo&cod=2&lan=zh&ctp=1&pdt=1&spd=5&per=4&vol=5&pit=5";
            sound += "&tex=";
            sound += "温度:";
            string ret = digitToChinese(atoi(jsonOutData["data"]["wendu"].asString().c_str()));
            sound += ret;
            sound += ",感冒:";
            sound += jsonOutData["data"]["ganmao"].asString();
            sound += "\"";
            //sprintf(sound.c_str(), "mplayer \"http://tts.baidu.com/text2audio?idx=1&cuid=baidu_speech_demo&cod=2&lan=zh&ctp=1&pdt=1&spd=5&per=4&vol=5&pit=5&tex=温度%d,感冒:%s", jsonOutData["data"]["wendu"].asFloat(), jsonOutData["data"]["ganmao"].asString().c_str());
            printf("%s\n", sound.c_str());
            system(sound.c_str());
            return true;
        }
    }
    printf("失败 入参 %s, 出参%s",  strIndata.c_str(), strOutData.c_str());
    return false;
}





bool mojiRegexSearch(std::string html)
{
    std::string pattern("wea_alert clearfix[\\s\\S]*?<em>(\\d*).*</em>[\\s\\S]*?wea_weather clearfix[\\s\\S]*?<em>(.*)</em>[\\s\\S]*?<b>(.*)</b>[\\s\\S]*?湿度 (\\d*)%</span>[\\s\\S]*?<em>(.*)</em>[\\s\\S]*?<em>(.*)</em>");
    std::regex r(pattern);

    if (regex_search(html,results,r)){
        for (unsigned i=0; i<results.size(); ++i) {
            std::cout << "match " << i << ": " << results[i];
            std::cout << " (with a length of " << results[i].length() << ")\n";
        }
        return true;
    }
    return false;
}


bool mojiWeatherAPI(){
    string strOutData;
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value jsonOutData;

    string strIndata("http://tianqi.moji.com/");

    int CURLCode = Get(strIndata, strOutData);
    if(CURLCode == CURLE_OK){//成功
        return mojiRegexSearch(strOutData);
    }
    printf("失败 入参 %s, 出参%s",  strIndata.c_str(), strOutData.c_str());
    return false;
}



