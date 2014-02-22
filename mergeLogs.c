//
//  main.c
//  MergeServerLogs
//
//  Created by peter blay on 01/08/2013.
//  Copyright (c) 2013 peter blay. All rights reserved.
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 10000
#define TIMESTAMPLEN 23

//struct for returning switches applied at command line. 
typedef struct{
    bool output; //0 for stdout, 1 for file output
    const char *outputFile; //if output==0, outputFile would normally be NULL. Not a problem if not though. 
    const char *sscLog1;
    const char *sscLog2;
    bool order; //1 for ascending (default), 0 for descending
    bool fail; //0 = OK, 1 = something's wrong with the command line string
    //in hindsight, this is fairly redundant. Could just call summary(); instead
    //Could potentially used to stop memory leaks etc,but think that's about it, so won't change. 
} switches;

void summary(const char * argv[]);
switches parseCLI(const char * args[], int argLen);
bool currentLeap(int year);
int numLeaps(int year);
unsigned long parseTime(char *time);
unsigned long searchForTimestamp(char *line);



int main(int argc, const char * argv[])
{
    switches s;
    
    s=parseCLI(argv, argc);

    //really the sscLog2==NULL check is currently redundant. Just in case I change other functionality in the future.
    if (s.fail || s.sscLog1==NULL || s.sscLog2==NULL)
    {
        summary(argv);
    } else if (s.sscLog1==s.sscLog2)
    {
        printf("Log files specified are the same. Merging not necessary.");
        exit(EXIT_FAILURE);
    }

    FILE *log1 = fopen(s.sscLog1, "r");
    if (log1==NULL)
    {
        printf("There was a problem opening the file %s\n", s.sscLog1);
        exit(EXIT_FAILURE);
    }
    FILE *log2 = fopen(s.sscLog2, "r");
    if (log2==NULL)
    {
        printf("There was a problem opening the file %s\n", s.sscLog2);
        exit(EXIT_FAILURE);
    }

    char *line1 = NULL;
    char *line2 = NULL;
    size_t len1 = 0;
    size_t len2 = 0;
    ssize_t read1;
    ssize_t read2;

    unsigned long log1TS = 0;
    unsigned long log2TS = 0;

    /*
        Best way to go through without using multi-threading: 
        -search for timestamps in each
        -determine which came first, then read from that
        -when find another timestamp, write to output
        -note new timestamp and compare again

        With multi-threading: 
        -open new thread for each file
        -search for timestamp in each
        -when timestamp found in each (wait on file that doesn't ..making sure not to wait forever), 
        then compare
        -for earliest, print until a new timestamp is found
        -when new timestamp found, compare with other and again print earlier. 

        Advantage of multi-threading: during initial search for timestamp, also perhaps small advantage in other parts when starting/stopping. 
        --thinking further, there may be logic simplicity advantages with multi-threading
    */

    //GET INITIAL TIMESTAMPS
    
    while ((read1 = getline(&line1, &len1, log1))!=-1)
    {
        
        //printf("Received line of length %zu :\n", read);
        log1TS = searchForTimestamp(line1);
        //printf("Straight away log1TS is: %lu\n", log1TS);
        if (log1TS ==0)
        {
            printf("%s", line1);    
        }else{
            while ((read2 = getline(&line2, &len2, log2))!=-1)
            {
                log2TS = searchForTimestamp(line2);
                
                if (log2TS==0)
                {
                    printf("%s", line2); 
                    
                }else{
                    //break out of this loop when first timestamp encountered
                    break;   
                }
                
            }
            //at this point should have timestamps for first and second log, so break out of loop
            break;
        }

        
    }
    while(true)
    {
        //TOFIX: Looks to be correct, need to verify
        
        //if at end of both files

        if ((read1==-1) && (read2==-1))
        {
            break;
        }else if (read1==-1)
        {
            //if log1 finished but log2 not, just show the rest of log2
            do
            {
                printf("%s", line2);
            }while ((read2 = getline(&line2, &len2, log2))!=-1);
        }else if (read2==-1)
        {
            //if log2 finished but log2 hasn't, print the rest of log1
            do
            {
                printf("%s", line1);
            }while ((read1 = getline(&line1, &len1, log1))!=-1);
        }
        if (log1TS < log2TS)
        {
            //print line containing timestamp
            printf("%s", line1);
            while ((read1 = getline(&line1, &len1, log1))!=-1)
            {
                
                log1TS = searchForTimestamp(line1);
                if (log1TS ==0)
                {
                    //ALLOW
                    printf("%s", line1);    
                }else{
                    break;
                }
            }
        }else if (log1TS >= log2TS){
            //print line containing timestamp
            printf("%s", line2);
            while ((read2 = getline(&line2, &len2, log2))!=-1)
            {
                //printf("log2 loop\n");
                log2TS = searchForTimestamp(line2);
                //printf("Straight away log1TS is: %lu\n", log1TS);
                if (log2TS ==0)
                {
                    printf("%s", line2);    
                }else{
                    break;
                }
            }
        }
    }

    free(line1);
    free(line2);

    fclose(log1);
    fclose(log2);
    

    exit(EXIT_SUCCESS);
    
}

/**
Parses the line searching for the timestamp (at the start of the line)
@param line: the pointer to the line 
@return returns the long value of of the date for easy comparison. 
*/
unsigned long searchForTimestamp(char *line)
{
    /*
    Only BASIC functionality: just looks at the first TIMESTAMPLEN characters of the string and parses.
    */
    char *tsEnd;
    unsigned long ts;
    char *timestamp = NULL;
    
    timestamp = malloc(TIMESTAMPLEN+1);
    tsEnd = stpncpy(timestamp, line, TIMESTAMPLEN);
    *tsEnd = '\0';
    ts = parseTime(timestamp);
    free(timestamp);
    
    return ts;

}

/**
 Parses the command line arguments passed to the program.
 @param args: these are the arguments passed to the command line. It's what need to be parsed.
 @return returns a struct carrying the info about the command line switches
 */
switches parseCLI(const char * args[], int argLen)
{
    switches s = {0, NULL, NULL, NULL, true, false};
    bool tooManyLogs=false;
    int i;
    for (i = 1; i < argLen; i++)
    {
        //go through looking for '-' as first char of argument
        if (*args[i]=='-')
        {
            /**
                TODO: MAKE SURE ONLY SWITCH IS ONLY 2 CHARS LONG!
            */
            if (*(args[i]+1)=='h')
                summary(args);
            if (*(args[i]+1)=='d')
                s.order=false;
            if (*(args[i]+1)=='f')
            {
                s.output=1;
                //output to separate file. Need to check existence of next argument
                if ((i==(argLen-1))||*args[i+1]=='-')
                {
                    //-f was last argument, incorrect usage.
                    //if argument after -f is another switch, incorrect usage. 
                    s.fail=true;
                    //no need to carry on
                    return s;
                }
                else
                {
                    s.outputFile=args[++i];
                    //want to increment i, as now have no reason to go over next argument. 
                }
            }
        }
        else
        {
            //should only come into the else once during the running. 
            //if comes in twice, i.e. 4 logs specified, then it will use the last 2. This shouldn't happen. 
            if (tooManyLogs)
            {
                s.fail=true;
                return s;
            }
            //now won't let it in again
            tooManyLogs=true;

            //should be the two log files
            if (i==(argLen-1))
            {
                //
                s.fail=true;
                return s;
            }
            s.sscLog1=args[i];
            //need to check next next thing to see if it's a switch. If so, FAIL!
            i++;
            if (*args[i]=='-')
            {
                s.fail=true;
                return s;
            }
            s.sscLog2=args[i];   
        }
    }
    return s;
}

/**
Is current year a leap year? This returns the answer, taking the year as a param
*/
bool currentLeap(int year)
{
    //if leap year
    if (!(year%400))
    {
        return true;
    }
    else if (!(year%4) && (year%100)) 
    {
        return true;
    }
    else
    {
        return false;
    }


}

/**
This tries to find the number of years between the year given and the year of the epoch (1970)
@param year: The year that you want to find between that and the epoch
@return: the number of leap years
*/
int numLeaps(int year)
{
    int leaps = 0;
    int compare = 1970;
    for (;compare < year; compare++)
    {
        if (currentLeap(compare))
            leaps++;
    }
    return leaps;

}

/**
@param: time format in: 
YYYY-MM-DD HH:mm:SS,mimimi
where mi is a millisecond
@return is the unsigned long value of the time after converted. 
TOFIX: Perhaps add some debugging
*/
unsigned long parseTime(char *time)
{
    unsigned long longTime = 0;
    //take year & convert to seconds
    
    //year
    char * startptr;
    startptr = &time[0];

    char* endptr;
 
    unsigned long year = strtoul(startptr, &endptr, 10);
    
    //month
    startptr = &time[5];
    unsigned long month = strtoul(startptr, &endptr, 10);
    if (month == 0 || month>12)
    {
        //Invalid timestamp & therefore return 0
        return 0;
    }

    //day
    startptr = &time[8];
    unsigned long day = strtoul(startptr, &endptr, 10);
    if (startptr==endptr)
    {
        //invalid day
        return 0;
    }
    
    //hour
    startptr = &time[11];
    unsigned long hour = strtoul(startptr, &endptr, 10);
    if (startptr==endptr || hour>23)
    {
        return 0;
    }

    //mins
    startptr = &time[14];
    unsigned long min = strtoul(startptr, &endptr, 10);
    if (startptr==endptr || min>59)
    {
       return 0;
    }

    //secs
    startptr = &time[17];
    unsigned long sec = strtoul(startptr, &endptr, 10);
    if (startptr==endptr || sec>59)
    {
        return 0;
    }

    //milliseconds
    startptr = &time[20];
    unsigned long millisecond = strtoul(startptr, &endptr, 10);
    if (startptr==endptr || millisecond>999)
    {
        return 0;
    }

    /* could be useful for debugging at some point if changed.
    printf("Created timestamp is: %04lu-%02lu-%02lu %02lu:%02lu:%02lu,%03lu\n", year, month, day, hour, min, sec, millisecond);
    */

    int dayMonths[] = {31 /*JAN*/, 28 /*FEB*/, 31 /*MAR*/, 30 /*APR*/, 31 /*MAY*/, 30 /*JUN*/, 31 /*JUL*/, 31 /*AUG*/, 30 /*SEP*/, 31 /*OCT*/, 30 /*NOV*/, 31 /*DEC*/};

    /* TODO: Possibly better to have a hashtable/dictionary, but it's a bit of a hassle in C. Not sure on any/much performance improvement considering use.
    */

    //longTime = (longTime - 1970) * how many seconds in a year
    //seconds in a day = 1 * 60 (1 min) * 60 (1 hour) * 24 (1 day)
    //seconds in a year = seconds in a day * 365 (1 year)
    //add a day for leap years
    int secsInAMinute = 60;
    int secsInAnHour = 60 * secsInAMinute;
    int secsInADay = secsInAnHour * 24;
    int secsInAYear = secsInADay * 365;
    int epochDiff = year-1970;
    if (epochDiff<0)
    {
        
        printf("Invalid year value found while parsing date: %s\n", time);
        exit(EXIT_FAILURE);
        
    }
    //get number of leap years
    int leaps = numLeaps((int) year);
    longTime = (secsInAYear * epochDiff) + (leaps * secsInADay);

    //if current year is leap year, will have to be accounted for in month
    bool currentYearLeap = currentLeap((int) year);

    if (month==2 && day>29) //in array feb is down as 28 days. Think would need large-ish code change for slightly better coding
    {
        printf("Invalid day value found while parsing date: %s\n", time);
    } else if (month==2 && day==29 && !currentYearLeap){  //if 29th Feb, make sure it's a leap year
        printf("Invalid day value found while parsing. Date shows 29th February on non-leap year\n");
    } else if (day>dayMonths[month-1] && month !=2){ //february already accounted for
        printf("Invalid day value found while parsing date: %s\n", time);
    }
    
    int i;
    for (i=0; i<month-1; i++){
        longTime += secsInADay * dayMonths[i];
    }
    //account for 29 days in february
    if (currentYearLeap && month > 2)
        longTime += secsInADay;

    //add seconds for previous days in month
    longTime += secsInADay * (day-1);

    //add seconds in hours
    longTime += secsInAnHour * hour;

    //add seconds in minutes
    longTime += secsInAMinute * min;

    //add individual seconds
    longTime += sec;

    //convert to milliseconds & add milliseconds
    longTime *= 1000;
    longTime += millisecond;

    return longTime;
 } 

/**
 To start with will just take the 2 log files, and print either to stdout, or
 an optional output file
 @param args: This is the arguments of the program. Only really needed for args[0] at the moment. 
 */
void summary(const char * args[])
{
    printf("Usage: \t%s [Options] <filename> <filename2>\n", args[0]);
    printf("\t%s ssc.log ssc_audit.log\t\t\tMerges the two logs by timestamp ascending. Printing to stdout. \n", args[0]);
    printf("\t%s -f mergedLogs.log ssc.log ssc_audit.log\tMerges the the two logs by timestamp asending. Outputs to mergedLogs.log\n", args[0]);
    printf("\t%s -d ssc.log ssc_audit.log\t\t\tMerges the two logs by timestamp in descending order.\n", args[0]);
    printf("\t%s -h\t\t\t\t\t\tPrints this message\n", args[0]);
    printf("\nThis program takes two logs and merges them. By default in ascending order, although this can be reverse with -d. By default the merged logs will be printed to standard output, although this can be outputted to a file with the -f switch.\n");
    /*
    other functionality to think about: 
    -rolling logs, what size to roll on? 
    -excluding logs until they match the same time frame? This could be helpful stripping the ssc_audit.log which is usually older
    -RTA logs?
    -ability to expand with custom log timestamps? (Perhaps look at WI/WIE/AMP or even arcsight logs?). 
    --this should be slightly easier with the way parseTime has been created, so strtoul parses with variables for start & stop positions. 
    --should make fairly simple to parse a new format, and apply it. However, wouldn't currently be possible to merge logs with different timestamp formats
    --this could be fairly simple to implement with just a separate implementation of the function for each log file. 
    --This change could then make it easier to implement a change to utilise merging > 2 logs at a time. Considering the ease of scripting I won't bother with this though. 
    */
    exit(EXIT_SUCCESS);
}



