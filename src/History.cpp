#include <Arduino.h>
#include "History.h"
#include "UDPLogger.h"

extern UDPLogger loggit;

History::History(unsigned int size)
{
    length = size;
    data = new  unsigned int[size+1];
    ma_data = new  unsigned int[size+1];
    current_index = 0;
    entries = 0;
}

//  Increment the iterator, and wrap back to zero
void History::inc(void)
{
    current_index = current_index + 1;
    if ( current_index == length )
        current_index = 0;
}
// Add a new element, overwriting old elements of more than 'size' elements have been added previously
void History::add(int d)
{
    data[current_index] = d;
    inc();
   
    if ( entries < length )
        entries = entries + 1;
}
// calculate the average of all of the entries
float History::average(void)
{
    int total=0;
    int l;

    l =  entries < length ? entries : length;
 
    for ( int i = 0 ; i <  l ; i++)
        total = total + data[i];

    return total/l;
    
}
unsigned int History::getHistory(unsigned int events, unsigned int *result, unsigned int *max)
{
    int index = 0;
    int m = 0;
    if ( entries < events )
        events = entries;
    
    for ( int i = 0; i < events ; i++)
        result[i]=0;
    
    index = current_index -1;
    if ( index < 0 )
        index = length -1;

    for ( int i = 0 ; i < events ; i++ )
    {
            *result = data[index];
            if ( data[index] > m )
                m = data[index];

            index = index -1;
            if ( index < 0 )
                index = length - 1;
            result = result + 1;
    }
    *max = m;
    return events;
}
int History::last()
{
    int index;
    int answer =0;
    if ( entries  > 0 )
    {
        index = current_index - 1 ;
        if ( index < 0 )
            index = length - 1;
        answer =  data[index];
    }
    return answer;
}
//claculate the moving avregae of the last 'smaples' number of samples
float History::moving_average(unsigned int samples)
{
    int index;
    int total=0;
    if ( entries < samples )
        return 0;

    if ( samples > entries )
        samples = entries;
    // total the 'samples' previous entries
    index = current_index - 1 ;
    if ( index < 0 )
        index = length - 1;

    for ( unsigned int i = 0 ; i < samples ; i++ )
    {
        total = total + data[index];
        index = index - 1;
        if ( index < 0 )
            index = length - 1;
    }

    return total/samples;

}
//  Add HTML emphasis by turning the font red if the value is over 200
String History::emphasis(int threshold, int d)
{
    String answer;
    if ( d > threshold )
      answer = String("<font color='red'>") + String(d) + String("</font>");
    else
      answer = String(d);

    return answer;
}
unsigned int History::num_entries(void)
{
    if ( entries < length )
        return entries;
    else
    {
        return length;
    }
    
}
// Ceate a string with a list of values read from the sensor history.
String History::list()
{
    int index;
    int num;
    String result("[");

    if ( entries < length )
    {
        index = 0;
        num = entries;
    }
    else
    {
        index = current_index;
        num = length;
    }
    for ( int i = 0 ; i < num;i++)
    {
        if ( (i + 1) == num  )
            result = result +  data[index];
        else
            result = result +  data[index]  + String(",");
            
        
        index = (index + 1 ) % length;
    }
    result = result + "]";
    return result;
    
}

