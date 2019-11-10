#ifndef History_h
#define History_h


class History {
    public:
        History(unsigned int the_size=10);
        void add(int d);
        float average(void);
        int last(void);
        float moving_average(unsigned int samples);
        String list(int threshold);
        unsigned int getHistory(unsigned int len,unsigned int *data,unsigned int *max);
    private:
        void inc(void);
        String emphasis(int threshold,int value);
        unsigned int length;  // how big is the data array created on construction
        unsigned int *data;   // the data array of integers
        unsigned int *ma_data;   // the moving avegrage data array of integers
        unsigned int current_index;  // where we will put the next data item added
        unsigned int entries;  // how many entries have been added. Only relavent when its less than length in average calculations.
};
#endif