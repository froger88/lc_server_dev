/* 
 * File:   cstatistics.h
 * Author: froger
 *
 * Created on 22 wrzesień 2009, 22:59
 */

#ifndef CSTATISTICS_H
#define	CSTATISTICS_H

#include <string>
using namespace std;

#include "main.h"
#include "settings.h"
#include "mrbug.h"
#include "inttostr.h"

class CStatistics
{
public:
  CStatistics(int size = 5000);
  CStatistics(const CStatistics& orig);
  virtual ~CStatistics();

  void add(string IP, int TimeIn, int TimeOut, string Browser, string Page, string Ref_Page, string System, string Language, string Resolution, string Aim, string Returning, string City, string Country, string Date);
  void saveAndClean(int MaxMrBug);

private:

  string *ip;
  int *timeIn;
  int *timeOut;
  int *views;

  string *browser;
  string *page;
  string *ref_page;
  string *system;
  string *language;
  string *resolution;
  string *aim;
  string *returning; //"0" = new, "1"=returning
  string *date;
  string *city;
  string *country;

  bool *to_save;
  int *exists;

  int size;

};

#endif	/* CSTATISTICS_H */