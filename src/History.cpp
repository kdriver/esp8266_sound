#include <Arduino.h>
#include "History.h"

History::History(int size)
{
    length = size;
    data = new int[size];
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
// calculate the average from the last 'smaples' entries.
int History::moving_average(int samples)
{
    int index;
    int total=0;
    if ( samples > length )
        samples = length;
    // total the 'samples' previous entries
    index = current_index - 1 ;
    if ( index < 0 )
        index = length - 1;

    for ( int i = 0 ; i < samples ; i++ )
    {
        total = total + data[index];
        index = index - 1;
        if ( index < 0 )
            index = length - 1;
    }

    return total/samples;

}
String History::emphasis(int d)
{
    String answer;
    if ( d > 200 )
      answer = String("<font color='red'>") + String(d) + String("</font>");
    else
      answer = String(d);

    return answer;
}
String History::list()
{
    int index;
    int num;
    String result("<p>");

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
        result = result + emphasis(data[index]) + String(",");
        index = (index + 1 ) % length;
    }
    result = result + "</p>";
    return result;
    
}

