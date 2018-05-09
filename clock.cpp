#include <stdio.h>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <string>
#include <iostream>
#include <regex>

using namespace std;

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


char *pszNian[]={"","十","百","千","万","十","百","千","亿"};
char *pszShu[]={"零","壹","贰","叁","肆","伍","陆","柒","捌","玖"};
//char digitToChinese[200];

//digitToChinese
string dToC(int a,int b)
{
    static string digitToChinese;
    if (0 == b){
        digitToChinese.clear();
    }
    
	if(a>=10)
	{
		dToC(a/10,b+1);
	}
    digitToChinese += pszShu[a%10];
    digitToChinese += pszNian[b];
	//sprintf(digitToChinese, "%s%s%s",digitToChinese, pszShu[a%10],pszNian[b]);
    return digitToChinese;
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

//需要jsoncpp库
// bool sojsonWeatherAPI(){
// 	string strOutData;
//     Json::Reader reader; 
//     Json::FastWriter writer;
//     Json::Value jsonOutData; 
    
//     string strIndata("https://www.sojson.com/open/api/weather/json.shtml?city=%E6%A0%AA%E6%B4%B2");

//     int CURLCode = Get(strIndata, strOutData); //"\{\"car_num\":\"湘B12345\",\"parking_code\":\"C00002\"}"
//     if(strOutData[0] == '{'){//json字串
//         reader.parse(strOutData, jsonOutData);
//         if (jsonOutData["status"].asString() == "200"){//成功
//             //printf("成功 入参 %s, 出参%s", strIndata.c_str(), strOutData.c_str());
//             string sound("mplayer \"http://tts.baidu.com/text2audio?");
//             sound += "idx=1&cuid=baidu_speech_demo&cod=2&lan=zh&ctp=1&pdt=1&spd=5&per=4&vol=5&pit=5";
//             sound += "&tex=";
//             sound += "温度:";
//             dToC(atoi(jsonOutData["data"]["wendu"].asString().c_str()), 0);
//             sound += digitToChinese;
//             sound += ",感冒:";
//             sound += jsonOutData["data"]["ganmao"].asString();
//             sound += "\"";
//             //sprintf(sound.c_str(), "mplayer \"http://tts.baidu.com/text2audio?idx=1&cuid=baidu_speech_demo&cod=2&lan=zh&ctp=1&pdt=1&spd=5&per=4&vol=5&pit=5&tex=温度%d,感冒:%s", jsonOutData["data"]["wendu"].asFloat(), jsonOutData["data"]["ganmao"].asString().c_str());
//             printf("%s\n", sound.c_str());
//             system(sound.c_str());
//             return true;
//         }
//     }
//     printf("失败 入参 %s, 出参%s",  strIndata.c_str(), strOutData.c_str());
//     return false;
// }


// match 1: 22 优 (with a length of 6)
// match 2: 20 (with a length of 2)
// match 3: 阴 (with a length of 3)
// match 4: 87 (with a length of 10)
// match 5: 西北风2级 (with a length of 13)
// match 6: 冷热适宜，感觉很舒适。 (with a length of 33)
std::smatch results;

bool mojiRegexSearch(string html)
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

char strtime[100] = {0};
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


int old_minute = 0;

bool GetTime()
{
        time_t timep;
        time(&timep);
        tm *Local = localtime(&timep);

        if (Local->tm_min % 4 == 0 && 30 == Local->tm_sec && old_minute != Local->tm_min){
            system("mplayer \"heartbeat.mp3\"");
            old_minute = Local->tm_min;
        }
        if (7 == Local->tm_hour && 30 == Local->tm_min){
            string hour = dToC(Local->tm_hour, 0);
            string min = dToC(Local->tm_min, 0);
            string sec = dToC(Local->tm_sec, 0);
            sprintf(strtime, "%04d年%02d月%02d日 %s点%s分%s秒", Local->tm_year + 1900, Local->tm_mon + 1, Local->tm_mday, hour.c_str(), min.c_str(), sec.c_str());
            
            return true;
        }
        //sprintf(strtime, "%04d-%02d-%02d %02d:%02d:%02d", Local->tm_year + 1900, Local->tm_mon + 1, Local->tm_mday, Local->tm_hour, Local->tm_min, Local->tm_sec);
        
        return false;
}

// text = '早上好！今天是%s,天气%s,温度%s摄氏度,%s,%s,%s,%s' % today, weather, temp, sd, wind, aqi, info)
//http://tts.baidu.com/text2audio?idx=1&tex=早上好&cuid=baidu_speech_demo&cod=2&lan=zh&ctp=1&pdt=1&spd=5&per=4&vol=5&pit=5",
void PlayMusic(){
    string sound("mplayer \"http://tts.baidu.com/text2audio?");
    sound += "idx=1&cuid=baidu_speech_demo&cod=2&lan=zh&ctp=1&pdt=1&spd=5&per=4&vol=5&pit=5";
    sound += "&tex=";
    sound += "早上好！今天是";
    sound += strtime;
    sound += ",天气";
    sound += results[3];
    sound += ",温度";
    sound += dToC(atoi(results.str(2).c_str()), 0);
    sound += "摄氏度,湿度百分之";
    sound += dToC(atoi(results.str(4).c_str()), 0);
    sound += "，";
    sound += results[5];
    sound += "，空气质量";
    sound += dToC(atoi(results.str(1).c_str()), 0);
    sound += results[6];
    sound += "\"";
   
    printf("%s\n", sound.c_str());
    system("timeout 120 x-www-browser https://douban.fm/");
    system(sound.c_str());
    system(sound.c_str());
    system(sound.c_str());
}


int main(){

    while(1){
        if (GetTime()){
            if (mojiWeatherAPI()){
                PlayMusic();
            }
        }
        system("sleep 0.5");
    }

    return 0;
}