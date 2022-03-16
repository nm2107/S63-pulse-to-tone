#ifndef S63_DIALEDDIGIT_H
#define S63_DIALEDDIGIT_H

class DialedDigit
{
    public:
        DialedDigit();

        bool isNew() const;
        void push(unsigned int value);
        unsigned int flush();

    private:
        int value;
};

#endif
