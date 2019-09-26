#include <stdio.h>
#include <assert.h>
#include <string.h>

#define MAXDATA 10
#define POISON_CONST -666
#define stack_construct( this_ )    assert (&this_);                              \
                                    memset (this_.data, 0, MAXDATA);              \
                                    this_.size_ = 0;                              \
                                    this_.error = 0;                              \
                                    this_.destructed = 0;                         \
                                    int temp_i = 0;                               \
                                    for(; temp_i < sizeof (#this_) - 1; temp_i++) \
                                    {                                             \
                                        this_.name[temp_i] = #this_[temp_i];      \
                                    }                                             \
                                    this_.name[temp_i + 1] = '\0';                \
                                    assert (stack_ok (&this_))
typedef int elem_t;

struct stack_t
{
    elem_t data[MAXDATA];
    int size_;
    int error;
    int destructed;
    char name[];
};

void dump (const struct stack_t *this_)
{
    fputs (this_ -> name, stdout);
    printf ("\n%d\n", this_ -> error);
}

int stack_ok (struct stack_t *this_)
{
    if (&this_ == NULL)
    {
        this_ -> error = 1;
        dump (this_);
        return 0;
    }
    if (this_ -> destructed != 0)
    {
        this_ -> error = 2;
        dump (this_);
        return 0;
    }
    if (this_ -> size_ > MAXDATA - 1)
    {
        this_ -> error = 3;
        dump (this_);
        return 0;
    }
    if (this_ -> size_ < 0)
    {
        this_ -> error = 4;
        dump (this_);
        return 0;
    }
    if (this_ -> error != 0)
    {
        dump (this_);
        return 0;
    }
    dump (this_);
    return 1;
}

int stack_push (struct stack_t *this_, elem_t value)
{
    assert (stack_ok (this_));
    this_ -> size_++;
    if (this_ -> size_ > MAXDATA - 1)
    {
        this_ -> error = 3;
        return 0;
    }
    this_ -> data[this_ -> size_] = value;
    assert (stack_ok (this_));
    return 1;
}


int stack_pop (struct stack_t *this_)
{
    assert (stack_ok (this_));
    this_ -> size_--;
    if (this_ -> size_ < 0)
    {
        this_ -> error = 4;
        return POISON_CONST;
    }
    int ans = this_ -> data[this_ -> size_];
    this_ -> data[this_ -> size_] = POISON_CONST;
    assert (stack_ok (this_));
    return ans;
}

int stack_destruct (struct stack_t *this_)
{
    assert (stack_ok (this_));
    this_ -> destructed = 1;
    return 1;
}

int main ()
{
    struct stack_t stk1 = {};
    stack_construct (stk1);
    stack_push (&stk1, 10);
    int val = stack_pop (&stk1);
    stack_destruct (&stk1);
    printf("%d\n", val);
    return 0;
}
