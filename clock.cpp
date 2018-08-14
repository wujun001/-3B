#include <iostream>
#include <cstdlib>
#include <vector>
#include <regex>
#include <string>
#include <ctime>

#include "support.h"
#if WIN32
    #include <windows.h>
#endif
using namespace std;

extern std::smatch results;

struct AlarmTime
{
    int hour;
    int minute;
    bool onceADay;
};

vector<AlarmTime> g_vAlarmTime; //闹钟时间

int g_nBlueMusicBoxPreviousPlayMinute = 0; //避免蓝牙音箱自动关机
char g_szCurrentTime[100] = {0};

void initAlarmTime();
bool matchAlarmTime();
void getWeather();
void playWeather();
void playTime();
void playMusic();


int main(int argc, char *argv[])
{
    initAlarmTime();

    while(1)
    {
        if (true == matchAlarmTime())
        {
            getWeather();
            playTime();
            playWeather();
            playMusic();
        }
    #if WIN32
        Sleep(500);
    #else
        system("sleep 0.5");
    #endif
        cout << "clock is running!" << endl;
    }

    return 0;
}


void initAlarmTime()
{
    struct AlarmTime time1 = {7, 0, false};
    struct AlarmTime time2 = {7, 30, false};
    struct AlarmTime time3 = {20, 18, false};
    g_vAlarmTime.push_back(time1);
    g_vAlarmTime.push_back(time2);
    g_vAlarmTime.push_back(time3);
}


bool matchAlarmTime()
{
    time_t timep;
    time(&timep);
    tm *Local = localtime(&timep);

    //蓝牙间隙播放避免关机
    if (Local->tm_min % 4 == 0 && 30 == Local->tm_sec && g_nBlueMusicBoxPreviousPlayMinute != Local->tm_min){

#if WIN32
    cout << "蓝牙音箱心跳" << endl;
#else
    system("mplayer \"heartbeat.mp3\"");
#endif
        g_nBlueMusicBoxPreviousPlayMinute = Local->tm_min;
    }

    //清空一天一次标志
    if (Local->tm_hour == 0 && Local->tm_min == 0)
    {
        for (vector<AlarmTime>::iterator it = g_vAlarmTime.begin(); it != g_vAlarmTime.end(); it++ )
        {
            it->onceADay = false;
        }
    }

    //匹配时间
    for (vector<AlarmTime>::iterator it = g_vAlarmTime.begin(); it != g_vAlarmTime.end(); it++ )
    {
        if (it->hour == Local->tm_hour && it->minute == Local->tm_min && it->onceADay == false){
            it->onceADay = true;
            return true;
        }
    }
    return false;
}
void getWeather()
{
//    cout << "void getWeather()" << endl;
    mojiWeatherAPI();
}

// match 1: 22 优 (with a length of 6)
// match 2: 20 (with a length of 2)
// match 3: 阴 (with a length of 3)
// match 4: 87 (with a length of 10)
// match 5: 西北风2级 (with a length of 13)
// match 6: 冷热适宜，感觉很舒适。 (with a length of 33)
void playWeather()
{
    string sound("mplayer \"http://tts.baidu.com/text2audio?");
    sound += "idx=1&cuid=baidu_speech_demo&cod=2&lan=zh&ctp=1&pdt=1&spd=5&per=4&vol=5&pit=5";
    sound += "&tex=";
    sound += "早上好！今天是";
    sound += g_szCurrentTime;
    sound += ",天气";
    sound += results[3];
    sound += ",温度";
    sound += digitToChinese(atoi(results.str(2).c_str()));
    sound += "摄氏度,湿度百分之";
    sound += digitToChinese(atoi(results.str(4).c_str()));
    sound += "，";
    sound += results[5];
    sound += "，空气质量";
    sound += digitToChinese(atoi(results.str(1).c_str()));
    sound += results[6];
    sound += "\"";

    cout << sound << endl;

    system(sound.c_str());
    system(sound.c_str());
    system(sound.c_str());

}
void playTime()
{
    //cout << "playMusic()" << endl;
    time_t timep;
    time(&timep);
    tm *Local = localtime(&timep);
    string hour = digitToChinese(Local->tm_hour);
    string min = digitToChinese(Local->tm_min);
    string sec = digitToChinese(Local->tm_sec);
    sprintf(g_szCurrentTime, "%04d年%02d月%02d日 %s点%s分%s秒", Local->tm_year + 1900, Local->tm_mon + 1, Local->tm_mday, hour.c_str(), min.c_str(), sec.c_str());

}

// text = '早上好！今天是%s,天气%s,温度%s摄氏度,%s,%s,%s,%s' % today, weather, temp, sd, wind, aqi, info)
//http://tts.baidu.com/text2audio?idx=1&tex=早上好&cuid=baidu_speech_demo&cod=2&lan=zh&ctp=1&pdt=1&spd=5&per=4&vol=5&pit=5",
void playMusic(){
    system("timeout 120 x-www-browser https://douban.fm/");
}
