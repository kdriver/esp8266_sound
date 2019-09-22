#ifndef History_h
#define History_h

class History {
    public:
        History(int the_size=10);
        void add(int d);
        float average(void);
        int moving_average(int samples);
        String list(void);
    private:
        void inc(void);
        String emphasis(int);
        int length;  // how big is the data array created on construction
        int *data;   // the data array of integers
        int current_index;  // where we will put the next data item added
        int entries;  // how many entries have been added. Only relavent when its less than length in average calculations.
};
#endif