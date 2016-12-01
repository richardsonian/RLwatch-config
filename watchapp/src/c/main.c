 //!TODO
/*
NOTE: function to get daytype; call on day or if not exist when refrenced
*/

#include <pebble.h>
#include <string.h>

//Definitions
#define STORAGE_KEY_PERIOD_DATE 0
#define STORAGE_KEY_PERIOD_ZERO 1
#define STORAGE_KEY_PERIOD_ONE 2
#define STORAGE_KEY_PERIOD_TWO 3
#define STORAGE_KEY_PERIOD_THREE 4
#define STORAGE_KEY_PERIOD_FOUR 5
#define STORAGE_KEY_PERIOD_FIVE 6
#define STORAGE_KEY_PERIOD_SIX 7
#define STORAGE_KEY_PERIOD_SEVEN 8
#define STORAGE_KEY_PERIOD_EIGHT 9
#define STORAGE_KEY_PERIOD_NINE 10
#define STORAGE_KEY_PERIOD_TEN 11

#define STORAGE_KEY_CLASS_DATE 12
#define STORAGE_KEY_CLASS_A 13
#define STORAGE_KEY_CLASS_B 14
#define STORAGE_KEY_CLASS_C 15
#define STORAGE_KEY_CLASS_D 16
#define STORAGE_KEY_CLASS_E 17
#define STORAGE_KEY_CLASS_F 18
#define STORAGE_KEY_CLASS_G 19
#define STORAGE_KEY_CLASS_H 20

#define STORAGE_KEY_CURRENT_BLOCK_CACHE 21
#define STORAGE_KEY_CURRENT_CLASS_CACHE 22
#define STORAGE_KEY_CURRENT_PERIOD_CACHE 23
#define STORAGE_KEY_CURRENT_CLASSTIME_CACHE 24
#define STORAGE_KEY_IS_DRAWN 25

//period declarations
struct period {
  int period;
  char name[32];
  int startHours;
  int startMinutes;
  int endHours;
  int endMinutes;
  char block[2];
  //Tuple *tuple;
  char string[64];
  int isLastPeriod;
};
static struct period periods[11];
static char periodLastUpdatedDate[12];

//class declarations
struct class {
  //int lunch;
  char string[128];
  char day[8][24];
};
static struct class classes[8];
static int classes_parsed = 0;

//change for pebble color
#ifdef PBL_COLOR
        static int hasColor = 1;
#else
        static int hasColor = 0;
#endif

static int appMessageReady = 0;

//window declaration
static Window *s_main_window;
//layer declarations
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_block_layer;
static TextLayer *s_class_layer;
static TextLayer *s_period_layer;
static TextLayer *s_classtime_layer;

//Font declarations
static GFont roboto_bold_condensed_16;
static GFont roboto_bold_condensed_18;
static GFont roboto_bold_48;

//FUNCTIONS
static void parse_classes() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Parsing classes");
  const char delim = '|';
  char *string;
  char *temp_string;
  char buf[128];
  
  if(classes_parsed==0) {
    classes_parsed = 1;
    persist_read_string(STORAGE_KEY_CLASS_A, classes[0].string, sizeof(classes[0].string));
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "reading strings...class A string: %s", classes[0].string);
    persist_read_string(STORAGE_KEY_CLASS_B, classes[1].string, sizeof(classes[1].string));
    persist_read_string(STORAGE_KEY_CLASS_C, classes[2].string, sizeof(classes[2].string));
    persist_read_string(STORAGE_KEY_CLASS_D, classes[3].string, sizeof(classes[3].string));
    persist_read_string(STORAGE_KEY_CLASS_E, classes[4].string, sizeof(classes[4].string));
    persist_read_string(STORAGE_KEY_CLASS_F, classes[5].string, sizeof(classes[5].string));
    persist_read_string(STORAGE_KEY_CLASS_G, classes[6].string, sizeof(classes[6].string));
    persist_read_string(STORAGE_KEY_CLASS_H, classes[7].string, sizeof(classes[7].string));
    
    for(int block=0; block<8; block++) {
      if(classes[block].string) {
        //first day
        memset(buf, '\0', sizeof(buf));
        string = classes[block].string;
        temp_string = strchr(string, delim);
        strncpy(buf, string, (strlen(string)-strlen(temp_string)));
        strcpy(classes[block].day[0], buf);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "string left: %s, cut off string: %s, rest of cutoff string: %s", string, buf, temp_string);
        for(int day=1; day<7; day++) {//middle 6
          memset(buf, '\0', sizeof(buf));
          string = temp_string+1;
          temp_string = strchr(string, delim);
          strncpy(buf, string, (strlen(string)-strlen(temp_string)));
          strcpy(classes[block].day[day], buf);
          //APP_LOG(APP_LOG_LEVEL_DEBUG, "string left: %s, cut off string: %s, rest of cutoff string: %s", string, buf, temp_string);
        }
        //last day
        memset(buf, '\0', sizeof(buf));
        string = temp_string+1;
        strcpy(classes[block].day[7], string);
      }
    }
  }
}
static void save_classes(DictionaryIterator *iterator) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Saving classes");
  persist_write_string(STORAGE_KEY_CLASS_A, dict_find(iterator, MESSAGE_KEY_CLASS_A)->value->cstring);
  persist_write_string(STORAGE_KEY_CLASS_B, dict_find(iterator, MESSAGE_KEY_CLASS_B)->value->cstring);
  persist_write_string(STORAGE_KEY_CLASS_C, dict_find(iterator, MESSAGE_KEY_CLASS_C)->value->cstring);
  persist_write_string(STORAGE_KEY_CLASS_D, dict_find(iterator, MESSAGE_KEY_CLASS_D)->value->cstring);
  persist_write_string(STORAGE_KEY_CLASS_E, dict_find(iterator, MESSAGE_KEY_CLASS_E)->value->cstring);
  persist_write_string(STORAGE_KEY_CLASS_F, dict_find(iterator, MESSAGE_KEY_CLASS_F)->value->cstring);
  persist_write_string(STORAGE_KEY_CLASS_G, dict_find(iterator, MESSAGE_KEY_CLASS_G)->value->cstring);
  persist_write_string(STORAGE_KEY_CLASS_H, dict_find(iterator, MESSAGE_KEY_CLASS_H)->value->cstring);
  
  parse_classes();
}

static void get_schedule() {
  //APP_LOG(APP_LOG_LEVEL_INFO, "starting get_schedule");
  if(appMessageReady) {
    DictionaryIterator *out_iter;
    AppMessageResult result = app_message_outbox_begin(&out_iter);
    //APP_LOG(APP_LOG_LEVEL_INFO, "Initializing outbox");
    if(result == APP_MSG_OK) {
      dict_write_int(out_iter, MESSAGE_KEY_PURPOSE, 0, sizeof(int), true);
      result = app_message_outbox_send();
    } else {
      //APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
    }
  }
}
static void draw_schedule(int currentPeriod, int periodType, int minutesLeft, int lastPeriod) {
  int isDrawn = persist_read_int(STORAGE_KEY_IS_DRAWN);
  
  //draw block
  static char block_text_buf[2];
  char oldBlockText[2];
  persist_read_string(STORAGE_KEY_CURRENT_BLOCK_CACHE, oldBlockText, sizeof(oldBlockText));
  if(currentPeriod != -1) {
    snprintf(block_text_buf, sizeof(block_text_buf), "%s", periods[currentPeriod].block);
    //deleted: persist_write_string(STORAGE_KEY_CURRENT_BLOCK_CACHE, block_text_buf);
  }
  else {
    snprintf(block_text_buf, sizeof(block_text_buf), "Z");
  }
  if(isDrawn==0 || strcmp(block_text_buf, oldBlockText)!=0) {
    text_layer_set_text(s_block_layer, block_text_buf);
    persist_write_string(STORAGE_KEY_CURRENT_BLOCK_CACHE, block_text_buf);
  }
  
  //draw classtime
  static char classtime_text_buf[24];
  char oldClasstimeText[24];
  if(minutesLeft != -1 && (periodType == 0 || periodType == 1 || periodType == 4)) {
    int hours = minutesLeft/60;
    if(hours!=0) {snprintf(classtime_text_buf, sizeof(classtime_text_buf), "%d hr %d min Left", hours, minutesLeft%60);}
    else{snprintf(classtime_text_buf, sizeof(classtime_text_buf), minutesLeft==1 ? "%d Minute Left" : "%d Minutes Left", minutesLeft);}
  } else {
      if(periodType==2) {snprintf(classtime_text_buf, sizeof(classtime_text_buf), "Have a nice night!");}
      else if(periodType==3){snprintf(classtime_text_buf, sizeof(classtime_text_buf), "Have a nice weekend!");}
      else if(periodType==-1){snprintf(classtime_text_buf, sizeof(classtime_text_buf), "Have a nice day!");}
  }
  
  if(isDrawn==0 || strcmp(classtime_text_buf, oldClasstimeText)!=0) {
    text_layer_set_text(s_classtime_layer, classtime_text_buf);
    persist_write_string(STORAGE_KEY_CURRENT_CLASSTIME_CACHE, classtime_text_buf);
  }
  
  //draw period
  static char period_text_buf[24];
  char oldPeriodText[24];
     switch(periodType) {
       case -1:
         snprintf(period_text_buf, sizeof(period_text_buf), "No School");
         break;
       case 0:
         snprintf(period_text_buf, sizeof(period_text_buf), strlen(periods[currentPeriod].name)>3 ? "%s" : "%s Period", periods[currentPeriod].name);//not tested
         break;
       case 1:
         snprintf(period_text_buf, sizeof(period_text_buf), "Before School");
         break;
       case 2:
         snprintf(period_text_buf, sizeof(period_text_buf), "After School");
         break;
       case 3:
         snprintf(period_text_buf, sizeof(period_text_buf), "Weekend");
         break;
       case 4:
         snprintf(period_text_buf, sizeof(period_text_buf), "Passing Time- %s", periods[currentPeriod].name);
         break;
     }
  
  if(isDrawn==0 || strcmp(period_text_buf, oldPeriodText)!=0) {
    text_layer_set_text(s_period_layer, period_text_buf);
    persist_write_string(STORAGE_KEY_CURRENT_PERIOD_CACHE, period_text_buf);
  }
  
  //draw class
  static char class_text_buf[24];
  //char oldClassText;
  int block=periods[currentPeriod].block[0]-'A';
  int day=0;
  for(int i=0; i<11; i++) {
    if(periods[i].period==1) {
      day=periods[i].block[0]-'A';
    }
  }
  
  if(periodType!=0 && periodType!=4) {
    snprintf(class_text_buf, sizeof(class_text_buf), "No Class");
  } else {
    snprintf(class_text_buf, sizeof(class_text_buf), classes[block].day[day]);
  }
  text_layer_set_text(s_class_layer, class_text_buf);
  
  persist_write_int(STORAGE_KEY_IS_DRAWN, 1);
} //change persistent/isdrawn to static variable old trackers;  WILL WORK (this doesnt)
static void find_school_info() {
  time_t findSchool_temp = time(NULL);
  struct tm *tick_time = localtime(&findSchool_temp);
  int minutesTotal = (tick_time->tm_hour*60 + tick_time->tm_min);//-((9)*60+(40));//offset
  int period0StartMinutesTotal = periods[0].startHours*60 + periods[0].startMinutes;
  int startPeriodMinutesTotal;
  int endPeriodMinutesTotal;
  int endLastPeriodMinutesTotal;
  
  int lastPeriod=10;//default 10 for wont set lastperiod if it there are 10 periods
  int schoolEndMinutesTotal = 0;
  
  for(int i=0; i<11; i++) {
    if(periods[i].isLastPeriod==1) {
      lastPeriod = i;
      break;
    }
  }
  schoolEndMinutesTotal = (periods[lastPeriod].endHours)*60 + periods[lastPeriod].endMinutes;
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "MinutesTotal: %d, period0StartMinutesTotal: %d", minutesTotal, period0StartMinutesTotal);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "lastPeriodEndMinutesTotal: %d, lastPeriod: %d", schoolEndMinutesTotal, lastPeriod);
  
  int currentPeriod = -1;
  int minutesLeft = -1;
  int periodType = -1; //0=day,1=beforeSchool,2=afterschool,3=weekend, 4=passingtime, -1=noschool
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Day number: %d", tick_time->tm_wday);
  if(tick_time->tm_wday!=6 && tick_time->tm_wday!=0){ //if not weekend
    if(period0StartMinutesTotal!=0) { //if schedule exists (if there is school)
      if(period0StartMinutesTotal<=minutesTotal) {//if not before homeroom
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Not before Homeroom");
        if(schoolEndMinutesTotal>minutesTotal) { //if not after school
          APP_LOG(APP_LOG_LEVEL_DEBUG, "During school day");
          for(int period=0; period<11; period++) {
          startPeriodMinutesTotal = periods[period].startHours*60 + periods[period].startMinutes; //time when period starts
          endPeriodMinutesTotal = periods[period].endHours*60 + periods[period].endMinutes; //time when period ends
          if(period-1>=0) {endLastPeriodMinutesTotal = periods[period-1].endHours*60 + periods[period-1].endMinutes;} //time when last period ends
          else{endLastPeriodMinutesTotal = startPeriodMinutesTotal;} //start of period if no previos period
          
           if(minutesTotal>=endLastPeriodMinutesTotal && minutesTotal<endPeriodMinutesTotal) {//if endLastPeriod<now<endThisPeriod
            currentPeriod = period; //its this period!
            if(minutesTotal>=startPeriodMinutesTotal) { //if its actually the period
              periodType = 0;
              minutesLeft = endPeriodMinutesTotal - minutesTotal;
            } else { //or just passing time
                periodType = 4;
                minutesLeft = startPeriodMinutesTotal - minutesTotal;
                currentPeriod = period;
              }
              break;
             }
          }
        } else { //else after school
          periodType = 2;
        }
          
      } else { //else before homeroom
          periodType = 1;
          minutesLeft = period0StartMinutesTotal - minutesTotal;
      }
    } else {//else no schedule (no school)
          periodType=-1;
      }
  } else {periodType = 3;}//else weekend
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "currentPeriod: %d, periodType: %d, minutesLeft: %d", currentPeriod, periodType, minutesLeft);
  draw_schedule(currentPeriod, periodType, minutesLeft, lastPeriod);
}//shifted
static void parse_schedule() {
  const char delim = ':';
  char *string;
  char *temp_string;
  char buf[32];
  
  time_t temp_parseSchedule = time(NULL)-3600;//sets an hour back (also in save_schedule)
  struct tm *tick_time = localtime(&temp_parseSchedule);
  APP_LOG(APP_LOG_LEVEL_INFO, "parsing time is %d:%d", tick_time->tm_hour, tick_time->tm_min);
  char currentDate[12];
  strftime(currentDate, sizeof(currentDate), "%x", tick_time);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "currentParseDate: %s", currentDate);
  
  char storagePeriodDate[12];
  persist_read_string(STORAGE_KEY_PERIOD_DATE, storagePeriodDate, sizeof(storagePeriodDate));
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "About to tell date read from period storage");
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Date in period storage: %s", storagePeriodDate);
  
  //check if stored data updated
  if(strcmp(currentDate, storagePeriodDate)!=0) { //set to false to never update from phone
    APP_LOG(APP_LOG_LEVEL_DEBUG, "stored data outdated or nonexistent. Retrieving...");
    get_schedule();
    return;
  }
  
    //check if variable data is updated
    if(strcmp(currentDate, periodLastUpdatedDate)!=0) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "variable data outdated or nonexistent. Retrieving...");
      
      //update lastUpdated Date
      strcpy(periodLastUpdatedDate, currentDate);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "lastPeriodUpdate now is: %s", periodLastUpdatedDate);
      //get period Strings
      persist_read_string(STORAGE_KEY_PERIOD_ZERO, periods[0].string, sizeof(periods[0].string));
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Period0 string: %s", periods[0].string);
      persist_read_string(STORAGE_KEY_PERIOD_ONE, periods[1].string, sizeof(periods[1].string));
      persist_read_string(STORAGE_KEY_PERIOD_TWO, periods[2].string, sizeof(periods[2].string));
      persist_read_string(STORAGE_KEY_PERIOD_THREE, periods[3].string, sizeof(periods[3].string));
      persist_read_string(STORAGE_KEY_PERIOD_FOUR, periods[4].string, sizeof(periods[4].string));
      persist_read_string(STORAGE_KEY_PERIOD_FIVE, periods[5].string, sizeof(periods[5].string));
      persist_read_string(STORAGE_KEY_PERIOD_SIX, periods[6].string, sizeof(periods[6].string));
      persist_read_string(STORAGE_KEY_PERIOD_SEVEN, periods[7].string, sizeof(periods[7].string));
      persist_read_string(STORAGE_KEY_PERIOD_EIGHT, periods[8].string, sizeof(periods[8].string));
      persist_read_string(STORAGE_KEY_PERIOD_NINE, periods[9].string, sizeof(periods[9].string));
      persist_read_string(STORAGE_KEY_PERIOD_TEN, periods[10].string, sizeof(periods[10].string));
      
      //parse
      for(int i=0; i<11; i++) {periods[i].isLastPeriod=0;}//init period.isLastPeriod
      for(int period=0; period<11; period++) {
        if(periods[period].string && strcmp(periods[period].string, "noperiod")!=0) {
          memset(buf, '\0', sizeof(buf));
          string = periods[period].string;
          temp_string = strchr(string, delim);
          strncpy(buf, string, (strlen(string)-strlen(temp_string)));
          periods[period].period = atoi(buf);
          
          memset(buf, '\0', sizeof(buf));
          string = temp_string+1;
          temp_string = strchr(string, delim);
          strncpy(buf, string, (strlen(string)-strlen(temp_string)));
          strcpy(periods[period].name, buf);
          
          memset(buf, '\0', sizeof(buf));
          string = temp_string+1;
          temp_string = strchr(string, delim);
          strncpy(buf, string, (strlen(string)-strlen(temp_string)));
          periods[period].startHours = atoi(buf);
          
          memset(buf, '\0', sizeof(buf));
          string = temp_string+1;
          temp_string = strchr(string, delim);
          strncpy(buf, string, (strlen(string)-strlen(temp_string)));
          periods[period].startMinutes = atoi(buf);
          
          memset(buf, '\0', sizeof(buf));
          string = temp_string+1;
          temp_string = strchr(string, delim);
          strncpy(buf, string, (strlen(string)-strlen(temp_string)));
          periods[period].endHours = atoi(buf);
          
          memset(buf, '\0', sizeof(buf));
          string = temp_string+1;
          temp_string = strchr(string, delim);
          strncpy(buf, string, (strlen(string)-strlen(temp_string)));
          periods[period].endMinutes = atoi(buf);
          
          string = temp_string+1;
          strcpy(periods[period].block, string);
          
          //APP_LOG(APP_LOG_LEVEL_DEBUG, "PERIOD PARSED");
          //APP_LOG(APP_LOG_LEVEL_DEBUG, "PERIOD_%d: period: %d, name: %s, startHours: %d", period, periods[period].period, periods[period].name, periods[period].startHours);
          //APP_LOG(APP_LOG_LEVEL_DEBUG, "startMinutes: %d, endHours %d, endMinutes: %d, block: %s", periods[period].startMinutes, periods[period].endHours, periods[period].endMinutes, periods[period].block);
        }
        else{
          periods[period-1].isLastPeriod=1;
          break;
        }
      }
      
    }
  parse_classes();
  find_school_info();
}
static void save_schedule(DictionaryIterator *iterator) {
  /* persist_delete(STORAGE_KEY_PERIOD_ZERO);
  persist_delete(STORAGE_KEY_PERIOD_ONE);
  persist_delete(STORAGE_KEY_PERIOD_TWO);
  persist_delete(STORAGE_KEY_PERIOD_THREE);
  persist_delete(STORAGE_KEY_PERIOD_FOUR);
  persist_delete(STORAGE_KEY_PERIOD_FIVE);
  persist_delete(STORAGE_KEY_PERIOD_SIX);
  persist_delete(STORAGE_KEY_PERIOD_SEVEN);
  persist_delete(STORAGE_KEY_PERIOD_EIGHT);
  persist_delete(STORAGE_KEY_PERIOD_NINE);
  persist_delete(STORAGE_KEY_PERIOD_TEN); */
  persist_write_string(STORAGE_KEY_PERIOD_ZERO, dict_find(iterator, MESSAGE_KEY_PERIOD_ZERO)->value->cstring);
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "iterator string: %s", dict_find(iterator, MESSAGE_KEY_PERIOD_ZERO)->value->cstring); //Saves it correctly (Tested)
  persist_write_string(STORAGE_KEY_PERIOD_ONE, dict_find(iterator, MESSAGE_KEY_PERIOD_ONE)->value->cstring);
  persist_write_string(STORAGE_KEY_PERIOD_TWO, dict_find(iterator, MESSAGE_KEY_PERIOD_TWO)->value->cstring);
  persist_write_string(STORAGE_KEY_PERIOD_THREE, dict_find(iterator, MESSAGE_KEY_PERIOD_THREE)->value->cstring);
  persist_write_string(STORAGE_KEY_PERIOD_FOUR, dict_find(iterator, MESSAGE_KEY_PERIOD_FOUR)->value->cstring);
  persist_write_string(STORAGE_KEY_PERIOD_FIVE, dict_find(iterator, MESSAGE_KEY_PERIOD_FIVE)->value->cstring);
  persist_write_string(STORAGE_KEY_PERIOD_SIX, dict_find(iterator, MESSAGE_KEY_PERIOD_SIX)->value->cstring);
  persist_write_string(STORAGE_KEY_PERIOD_SEVEN, dict_find(iterator, MESSAGE_KEY_PERIOD_SEVEN)->value->cstring);
  persist_write_string(STORAGE_KEY_PERIOD_EIGHT, dict_find(iterator, MESSAGE_KEY_PERIOD_EIGHT)->value->cstring);
  persist_write_string(STORAGE_KEY_PERIOD_NINE, dict_find(iterator, MESSAGE_KEY_PERIOD_NINE)->value->cstring);
  persist_write_string(STORAGE_KEY_PERIOD_TEN, dict_find(iterator, MESSAGE_KEY_PERIOD_TEN)->value->cstring);
  
  char buffer[12];
  time_t temp = time(NULL)-3600;//sets an hour back (also in parse_schedule)
  struct tm *tick_time = localtime(&temp);
  strftime(buffer, sizeof(buffer), "%x", tick_time);
  persist_write_string(STORAGE_KEY_PERIOD_DATE, buffer);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Schedule saved on with periodDate of: %s ", buffer);
  
  parse_schedule();
}

static void update_time() {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating time");
  time_t time_temp = time(NULL);
  struct tm *tick_time = localtime(&time_temp);
  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? 
                                         "%H:%M" : "%l:%M", tick_time);
  
  text_layer_set_text(s_time_layer, s_time_buffer);
}
static void update_date() {
  time_t date_temp = time(NULL);
  struct tm *tick_time = localtime(&date_temp);
  
  static char s_date_buffer[24];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%A, %b %e", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void minute_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "Minute tick heard!");
  update_time();
  parse_schedule();
  if(tick_time->tm_hour==0) {
    update_date();
  }
}

static void main_window_load(Window *window) {
  //Get info about window 
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  //APP_LOG(APP_LOG_LEVEL_INFO, "width:%d, height:%d", bounds.size.w , bounds.size.h);
  
  //load fonts
  roboto_bold_condensed_16 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_BOLD_16));
  roboto_bold_condensed_18 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_BOLD_18));
  roboto_bold_48 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_48));
  
  //window background color
  window_set_background_color(window, hasColor ? GColorLightGray : GColorBlack);
  
  //block layer
  s_block_layer = text_layer_create(GRect(30, 55, (bounds.size.w-60), 60));
  text_layer_set_background_color(s_block_layer, hasColor ? GColorYellow : GColorClear);
  text_layer_set_text_color(s_block_layer, hasColor ? GColorBlack : GColorWhite);
  text_layer_set_font(s_block_layer, roboto_bold_48);
  text_layer_set_text_alignment(s_block_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_block_layer));
  
  //Time layer
  s_time_layer = text_layer_create(GRect(0, 15, bounds.size.w, 45)); //make layer
  text_layer_set_background_color(s_time_layer, hasColor ?  GColorPictonBlue : GColorClear);
  text_layer_set_text_color(s_time_layer, hasColor ? GColorBlack : GColorWhite);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  //Date layer
  s_date_layer = text_layer_create(GRect(0, 0, bounds.size.w, 20));
  text_layer_set_background_color(s_date_layer, hasColor ? GColorBrightGreen : GColorClear);
  text_layer_set_text_color(s_date_layer, hasColor ? GColorBlack : GColorWhite);
  text_layer_set_font(s_date_layer, roboto_bold_condensed_16);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  //class layer
  s_class_layer = text_layer_create(GRect(0, 110, bounds.size.w, 20));
  text_layer_set_background_color(s_class_layer, hasColor ? GColorRichBrilliantLavender : GColorClear);
  text_layer_set_text_color(s_class_layer, hasColor ? GColorBlack : GColorWhite);
  text_layer_set_font(s_class_layer, roboto_bold_condensed_18);
  text_layer_set_text_alignment(s_class_layer, GTextAlignmentCenter);
  text_layer_set_text(s_class_layer, "Loading...");
  layer_add_child(window_layer, text_layer_get_layer(s_class_layer));
  
  //period layer
  s_period_layer = text_layer_create(GRect(0, 130, bounds.size.w, 20));
  text_layer_set_background_color(s_period_layer, hasColor ? GColorVividViolet : GColorClear);
  text_layer_set_text_color(s_period_layer, hasColor ? GColorBlack : GColorWhite);
  text_layer_set_font(s_period_layer, roboto_bold_condensed_16);
  text_layer_set_text_alignment(s_period_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_period_layer));
  
  //classtime layer
  s_classtime_layer = text_layer_create(GRect(0, (bounds.size.h-20), bounds.size.w, 20));
  text_layer_set_background_color(s_classtime_layer, hasColor ? GColorRajah : GColorClear);
  text_layer_set_text_color(s_classtime_layer, hasColor ? GColorBlack : GColorWhite);
  text_layer_set_font(s_classtime_layer, roboto_bold_condensed_16);
  text_layer_set_text_alignment(s_classtime_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_classtime_layer));
}
static void main_window_unload(Window *window) {
  //destroy layers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_block_layer);
  text_layer_destroy(s_class_layer);
  text_layer_destroy(s_period_layer);
  text_layer_destroy(s_classtime_layer);
  
  //unload fonts
  fonts_unload_custom_font(roboto_bold_condensed_16);
  fonts_unload_custom_font(roboto_bold_condensed_18);
  fonts_unload_custom_font(roboto_bold_48);
  
  persist_write_int(STORAGE_KEY_IS_DRAWN, 0);
}

//AppMessage Callbacks
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *purpose_tuple = dict_find(iterator, MESSAGE_KEY_PURPOSE);
  if(purpose_tuple->value->int32 == 0) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "Message received: schedule");
    save_schedule(iterator);
  }
  else if(purpose_tuple->value->int32 == 1) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "Message received: appMessage Ready");
    appMessageReady=true;
    parse_schedule();
  } else if(purpose_tuple->value->int32 == 2) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Message received: classes updated");
      save_classes(iterator);
  }
}
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  //APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  //APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void init() {
  //create window
  s_main_window = window_create();
  
  //create window load/unload handlers
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  //show window on watch (animations enabled)
  window_stack_push(s_main_window, true);
  
  //set time from beginning
  update_time();
  update_date();
  
  //subscribe to ticks 
  tick_timer_service_subscribe(MINUTE_UNIT, minute_tick_handler);
  
  //Register AppMessage Callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  //AppMessage init
  const int inbox_size = 2048;
  const int outbox_size = 256;
  app_message_open(inbox_size, outbox_size);
}
static void deinit() {
  window_destroy(s_main_window);
}
int main(void) {
  init();
  app_event_loop();
  deinit();
}