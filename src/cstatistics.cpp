/* 
 * File:   cstatistics.cpp
 * Author: froger
 * 
 * Created on 22 wrzesień 2009, 22:59
 */

#include "cstatistics.h"
#include <string>
#include <pthread.h>
using namespace std;

extern Settings Configuration;
//extern pthread_mutex_t intToStr_mutex;                                        USUNIETO
extern pthread_mutex_t mutex;

CStatistics::CStatistics(int size)
{
  if(size < 100)
  {
    size = 100;
  }

  ip = new string [size];
  timeIn = new int [size];
  timeOut = new int [size];
  views = new int [size];

  browser = new string [size];
  page = new string [size];
  ref_page = new string [size];
  system = new string [size];
  language = new string [size];
  resolution = new string [size];
  aim = new string [size];
  returning = new string [size]; //"0" = new, "1"=returning
  exists = new int [size];
  to_save = new bool [size];
  date = new string [size];
  city = new string [size];
  country = new string [size];

  //uzupelnianie domyslnymi wartosciami
  for(int i = 0; i < size; i++)
  {
    ip[i].clear();
    timeIn[i] = 0;
    timeOut[i] = 0;
    views[i] = 0;

    browser[i].clear();
    page[i].clear();
    ref_page[i].clear();
    system[i].clear();
    language[i].clear();
    resolution[i].clear();
    aim[i].clear();
    returning[i] = "not_set";
    exists[i] = 0;
    to_save[i] = false;
    date[i].clear();
    city[i].clear();
    country[i] = "";
  }

  this->size = size;
}

CStatistics::CStatistics(const CStatistics& orig)
{
}

CStatistics::~CStatistics()
{
  delete [] ip;
  delete [] timeIn;
  delete [] timeOut;
  delete [] views;

  delete [] browser;
  delete [] page;
  delete [] ref_page;
  delete [] system;
  delete [] language;
  delete [] resolution;
  delete [] aim;
  delete [] returning; //"0" = new, "1"=returning
  delete [] exists;
  delete [] to_save;
  delete [] date;
  delete [] city;
  delete [] country;

  ip = NULL;
  timeIn = NULL;
  timeOut = NULL;
  views = NULL;

  browser = NULL;
  page = NULL;
  ref_page = NULL;
  system = NULL;
  language = NULL;
  resolution = NULL;
  aim = NULL;
  exists = NULL;
  to_save = NULL;
  returning = NULL;
  date = NULL;
  city = NULL;
  country = NULL;
}

void CStatistics::add(string IP, int TimeIn, int TimeOut, string Browser, string Page, string Ref_Page, string System, string Language, string Resolution, string Aim, string Returning, string City, string Country, string Date)
{
  bool exists_in_db = false;
  for(int i = 0; i < size; i++)
  {
    if(ip[i] == IP)
    {
      //dopisz
      //zwieksz viewsy
      views[i]++;
      //uaktualnij czas wejscia (tylko jesli nowszy
      if(timeIn[i] > TimeIn)
      {
        timeIn[i] = TimeIn;
        //uaktualnij timestamp
        date[i] = Date;
      }
      //uaktualnij czas wyjscia (tylko jesli starszy)
      if(timeOut[i] < TimeOut)
      {
        timeOut[i] = TimeOut;
      }
      //ustaw browser
      browser[i] = Browser;
      //ustaw page
      if(page[i].empty())
      {
        page[i] = "::" + Page + "::";
      }
      else
      {
        string tmpPage = "::" + Page + "::";
        if(page[i].find(tmpPage) == string::npos)
        {
          page[i] += Page + "::";
        }
      }
      //ustaw ref_page
      if(ref_page[i].empty())
      {
        ref_page[i] = Ref_Page;
      }
      //ustaw system
      system[i] = System;
      //ustaw language
      if(language[i].empty())
      {
        language[i] = Language;
      }
      //ustaw resolution
      if(resolution[i].empty())
      {
        resolution[i] = Resolution;
      }
      //ustaw aim
      if(aim[i].empty())
      {
        aim[i] = Aim;
      }
      else
      {
        aim[i] += "," + Aim;
      }
      //ustaw miasto
      if(City == "E")
      {
        city[i] = "undefined";
      }
      //uistaw kraj
      country[i] = Country;
      if(Country == "E")
      {
        country[i] = "00";
      }
      //oznacz jako istniejacy w bazie
      exists_in_db = true;
    }
  }

  if(exists_in_db == false)
  {
    for(int i = 0; i < size; i++)
    {
      if(ip[i].empty() == true)
      {
        //dopisz
        ip[i] = IP;
        //zwieksz viewsy
        views[i] = 1;
        //uaktualnij czas wejscia (tylko jesli nowszy
        timeIn[i] = TimeIn;
        //uaktualnij timestamp
        date[i] = Date;
        //uaktualnij czas wyjscia (tylko jesli starszy)
        timeOut[i] = TimeOut;
        //ustaw browser
        browser[i] = Browser;
        //ustaw page
        page[i] = "::" + Page + "::";
        //ustaw ref_page
        ref_page[i] = Ref_Page;
        //ustaw system
        system[i] = System;
        //ustaw language
        language[i] = Language;
        //ustaw resolution
        resolution[i] = Resolution;
        //ustaw aim
        aim[i] = Aim;
        //ustaw miasto
        city[i] = City;
        if(City == "E")
        {
          city[i] = "undefined";
        }
        //uistaw kraj
        country[i] = Country;
        if(Country == "E")
        {
          country[i] = "00";
        }
        //ustaw czy_powracajacy
        if(returning[i] == "not_set")
        {
          returning[i] = Returning;
        }
        break;
      }
    }
  }
  return;
}

void CStatistics::saveAndClean(int MaxMrBug)
{
  // wytypuj powtarzajace sie mrBugi
  for(int i = 0; i < size; i++)
  {
    if(ip[i].empty() == false)
    {
      exists[i]++;
    }

    for(int j = 0; j < MaxMrBug; j++)
    {
      if(ip[i] == Configuration.MrBugTab[j].getIP() && ip[i].empty() == false)
      {
        exists[i] = 0;
      }
    }
  }

  for(int i = 0; i < size; i++)
  {
    if(exists[i] >= 2)//jezeli nie pojawil sie 2x pod rzad
    {
      to_save[i] = true;
    }
  }

  string DBUsername = "root";
  string DBDataBase = "";
  string DBPassword = "";
  string DBHost = "";
  string DBQuery = "";
  string DBPrefix = Configuration.getDBPrefix();;

  // utworz polaczenie z baza danych
  pthread_mutex_lock(&mutex);
  DBPrefix = Configuration.getDBPrefix();
  DBDataBase = Configuration.getDbID();
  pthread_mutex_unlock(&mutex);
  // zapisz dane

  MYSQL Database;
  if(!mysql_init(&Database))
  {
    return;
  }
  else
  {
    if(mysql_real_connect(&Database, DBHost.c_str(), DBUsername.c_str(), DBPassword.c_str(), DBDataBase.c_str(), 0, NULL, 0))
    {
      //zapisz dane
      for(int i = 0; i < size; i++)
      {
        if(to_save[i] == true)
        {
          if(ip[i].empty() == false)
          {
            int time = timeOut[i] - timeIn[i];

            string timeStr = intToStr(time);
            string viewsStr = intToStr(views[i]);

            char * S = new char[page[i].length()*3 + 1];
            memset(S, 0, page[i].length()*3 + 1);
            mysql_real_escape_string(&Database, S, page[i].c_str(), page[i].length());
            page[i].assign(S);
            delete [] S;

            S = new char[browser[i].length()*3 + 1];
            memset(S, 0, browser[i].length()*3 + 1);
            mysql_real_escape_string(&Database, S, browser[i].c_str(), browser[i].length());
            browser[i].assign(S);
            delete [] S;

            S = new char[resolution[i].length()*3 + 1];
            memset(S, 0, resolution[i].length()*3 + 1);
            mysql_real_escape_string(&Database, S, resolution[i].c_str(), resolution[i].length());
            resolution[i].assign(S);
            delete [] S;

            S = new char[language[i].length()*3 + 1];
            memset(S, 0, language[i].length()*3 + 1);
            mysql_real_escape_string(&Database, S, language[i].c_str(), language[i].length());
            language[i].assign(S);
            delete [] S;

            S = new char[returning[i].length()*3 + 1];
            memset(S, 0, returning[i].length()*3 + 1);
            mysql_real_escape_string(&Database, S, returning[i].c_str(), returning[i].length());
            returning[i].assign(S);
            delete [] S;

            S = new char[city[i].length()*3 + 1];
            memset(S, 0, city[i].length()*3 + 1);
            mysql_real_escape_string(&Database, S, city[i].c_str(), city[i].length());
            city[i].assign(S);
            delete [] S;

            S = new char[country[i].length()*3 + 1];
            memset(S, 0, country[i].length()*3 + 1);
            mysql_real_escape_string(&Database, S, country[i].c_str(), country[i].length());
            country[i].assign(S);
            delete [] S;

            DBQuery = "INSERT INTO " + DBPrefix + "s_ip (ip, page,ref_page, system, browser, resolution, language, views, sTime, returning,city ,country , date) VALUES (\'";
            DBQuery += ip[i] + "\' , \'" + page[i] + "\', \'" + ref_page[i] + "\', \'" + system[i] + "\', \'" + browser[i] + "\', \'" + resolution[i] + "\', \'" + language[i];
            DBQuery += "\'," + viewsStr + ", " + timeStr + ", " + returning[i] + " , \'" + city[i] + "\', \'" + country[i] + "\', \'" + date[i] + "\')";
            int query_result = mysql_real_query(&Database, DBQuery.c_str(), (long unsigned int) strlen(DBQuery.c_str()));
            pthread_mutex_lock(&mutex);
            cout << "MYSQL_QUERY #1: "<<DBQuery.c_str() << " return "<< intToStr(query_result) << endl;
            pthread_mutex_unlock(&mutex);
          }
          ip[i].clear();
          timeIn[i] = 0;
          timeOut[i] = 0;
          views[i] = 0;

          browser[i].clear();
          page[i].clear();
          ref_page[i].clear();
          system[i].clear();
          language[i].clear();
          resolution[i].clear();
          aim[i].clear();
          returning[i] = "not_set";
          exists[i] = 0;
          to_save[i] = false;
        }
      }
      mysql_close(&Database);
      return;
    }
    else
    {
      //disconnect from database
      mysql_close(&Database);
      return;
    }
  }
  return;
}

