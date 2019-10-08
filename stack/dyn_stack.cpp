#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <typeinfo>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>

#define POISON_CONST -666
#define LOG_FILE "stack.log"
#define CHUNK 24 // must be >= 24

#define CANARY_FRONT     3134507241
#define CANARY_END       3727866566
#define BUF_CANARY_FRONT 2814172690
#define BUF_CANARY_END   3536563781

#define stack_construct( this_ )    assert (!(is_pointer_wrong (&this_)));                                                                                                                                                                               \
                                                                                                                                                                                                                                                         \
                                    this_.canary_front = CANARY_FRONT;                                                                                                                                                                                   \
                                    this_.canary_end   = CANARY_END;                                                                                                                                                                                     \
                                    this_.all_struc_data = (void *) calloc (CHUNK, 1);                                                                                                                                                                   \
                                    if (this_.all_struc_data == NULL)                                                                                                                                                                                    \
                                    {                                                                                                                                                                                                                    \
                                        dump (&this_);                                                                                                                                                                                                   \
                                        abort ();                                                                                                                                                                                                        \
                                    }                                                                                                                                                                                                                    \
                                    this_.buf_canary_front    = (unsigned int *) this_.all_struc_data;                                                                                                                                                   \
                                    *(this_.buf_canary_front)    = BUF_CANARY_FRONT;                                                                                                                                                                     \
                                    this_.all_struc_data_size = (int *) this_.all_struc_data + 1;                                                                                                                                                        \
                                    *(this_.all_struc_data_size) = CHUNK;                                                                                                                                                                                \
                                    this_.size_               = (int *) this_.all_struc_data + 2;                                                                                                                                                        \
                                    *(this_.size_)               = 0;                                                                                                                                                                                    \
                                    this_.univ_itt            = (int *) this_.all_struc_data + 3;                                                                                                                                                        \
                                    *(this_.univ_itt)            = 0;                                                                                                                                                                                    \
                                    this_.error               = (int *) this_.all_struc_data + 4;                                                                                                                                                        \
                                    *(this_.error)               = 0;                                                                                                                                                                                    \
                                    this_.whole_size          = (int *) this_.all_struc_data + 5;                                                                                                                                                        \
                                    *(this_.whole_size)          = (int) floor ((float) CHUNK / sizeof (elem_t)) - 6;                                                                                                                                    \
                                                                                                                                                                                                                                                         \
                                    stack_resize_while (&this_, 24 + (int) ceil ((float) sizeof (#this_) / sizeof (elem_t)), 0);                                                                                                                         \
                                                                                                                                                                                                                                                         \
                                    this_.name                = (char *) this_.all_struc_data + 24;                                                                                                                                                      \
                                    *(this_.whole_size) -= (int) ceil ((float) sizeof (#this_) / sizeof (elem_t));                                                                                                                                       \
                                    for (*this_.univ_itt = 0; *this_.univ_itt < sizeof (#this_); (*this_.univ_itt)++)                                                                                                                                    \
                                    {                                                                                                                                                                                                                    \
                                        *(this_.name + *(this_.univ_itt)) = #this_[*(this_.univ_itt)];                                                                                                                                                   \
                                    }                                                                                                                                                                                                                    \
                                    *(this_.univ_itt) = 0;                                                                                                                                                                                               \
                                    *(this_.whole_size) -= (int) ceil ((float) sizeof (unsigned int ) / sizeof (elem_t));                                                                                                                                \
                                                                                                                                                                                                                                                         \
                                    stack_resize_while (&this_, 24 + (int) ceil ((float) sizeof (#this_) / sizeof (elem_t)) +                                                                                                                            \
                                                        ((int) ceil ((float) sizeof (elem_t) / sizeof (unsigned int))) * sizeof (unsigned int) +                                                                                                         \
                                                        sizeof (unsigned int), 0);                                                                                                                                                                       \
                                                                                                                                                                                                                                                         \
                                    this_.data                = (elem_t *) this_.all_struc_data + (int) ceil ((float) 24 / sizeof (elem_t)) +                                                                                                            \
                                                                (int) ceil ((float) sizeof (#this_) / sizeof (elem_t));                                                                                                                                  \
                                                                                                                                                                                                                                                         \
                                    for (*(this_.univ_itt) = 0; *(this_.univ_itt) < *(this_.whole_size); (*(this_.univ_itt))++)                                                                                                                          \
                                    {                                                                                                                                                                                                                    \
                                        *(this_.data + *(this_.univ_itt)) = POISON_CONST;                                                                                                                                                                \
                                    }                                                                                                                                                                                                                    \
                                    *(this_.univ_itt) = 0;                                                                                                                                                                                               \
                                    this_.buf_canary_end =    (unsigned int *) this_.all_struc_data + (int) ceil ((float) 24 / sizeof (elem_t)) +                                                                                                        \
                                                              (int) ceil ((float) sizeof (#this_) / sizeof (elem_t)) +                                                                                                                                   \
                                                              (int) ceil ((float) (*(this_.whole_size) * sizeof (elem_t)) / sizeof (unsigned int));                                                                                                      \
                                                                                                                                                                                                                                                         \
                                    *(this_.buf_canary_end)      = BUF_CANARY_END;                                                                                                                                                                       \
                                    this_.cntrl_sum = control_sum (this_.all_struc_data, *(this_.all_struc_data_size)) * control_sum ((int *) &this_ + 2, 13 * 4) % 1000000;                                                                             \
                                                                                                                                                                                                                                                         \
                                    assert (stack_ok (&this_))

typedef int elem_t;

struct stack_t;

void dump (const struct stack_t *this_);

int control_sum (void *pointer, int bytes_cnt);

int is_pointer_wrong (void *pointer);

int stack_ok (struct stack_t *this_);

int stack_push (struct stack_t *this_, elem_t value);

int stack_pop (struct stack_t *this_);

int stack_size (struct stack_t *this_);

int stack_resize (struct stack_t *this_, int not_constr);

int stack_resize_while (struct stack_t *this_, int size_needed, int not_constr);

int stack_destruct (struct stack_t *this_);

enum error_enum {no_errors,
                 wrong_stack_pointer,
                 stack_was_destructed,
                 stack_is_overflowed,
                 stack_is_empty,
                 univ_itt_is_not_0,
                 wrong_all_struc_data_pointer,
                 wrong_all_struc_data_size_pointer,
                 wrong_size__pointer,
                 wrong_univ_itt_pointer,
                 wrong_error_pointer,
                 wrong_whole_size_pointer,
                 wrong_name_pointer,
                 wrong_data_pointer,
                 broken_front_struct_canary,
                 broken_end_struct_canary,
                 broken_front_buf_canary,
                 wrong_buf_canary_front_pointer,
                 broken_end_buf_canary,
                 wrong_buf_canary_end_pointer,
                 broken_cntrl_sum};

struct stack_t
{
    unsigned int canary_front;
    unsigned int cntrl_sum;
    unsigned int *buf_canary_front;
    void *all_struc_data;
    int *all_struc_data_size;
    int *size_;
    int *univ_itt;
    int *error;
    int *whole_size;
    char *name;
    elem_t *data;
    unsigned int *buf_canary_end;
    unsigned int canary_end;
};

void dump (const struct stack_t *this_)
{
    FILE *output = fopen (LOG_FILE, "a");
    char error_info[21][50] = {"no errors",
                              "wrong stack pointer",
                              "stack was destructed",
                              "stack is overflowed",
                              "stack is empty",
                              "stack_t.univ_itt is not zero ((int) 0)",
                              "wrong all_struc_data pointer",
                              "wrong all_struc_data_size pointer",
                              "wrong size_ pointer",
                              "wrong univ_itt pointer",
                              "wrong error pointer",
                              "wrong whole_size pointer",
                              "wrong name pointer",
                              "wrong data pointer",
                              "broken canary_front of struct",
                              "broken canary_end of struct",
                              "broken buf_canary_front",
                              "wrong buf_canary_front pointer",
                              "broken buf_canary_end",
                              "wrong buf_canary_end pointer",
                              "broken cntrl_sum"};
    fprintf (output, "%s %s [%p] (%s)\n{\n", typeid (this_).name (), this_ -> name, this_,
                                             *(this_ -> error) == no_errors ? "OK" : "ERROR!!!");
    fprintf (output, "\terror = %d (%s)\n", *(this_ -> error), error_info[*(this_ -> error)]);
    fprintf (output, "\tdata[%d] = [%p]\n", *(this_ -> whole_size), this_ -> data);
    fprintf (output, "\t{\n");
    printf ("%s %s [%p] (%s)\n{\n", typeid (this_).name (), this_ -> name, this_,
                                    *(this_ -> error) == no_errors ? "OK" : "ERROR!!!");
    printf ("\terror = %d (%s)\n", *(this_ -> error), error_info[*(this_ -> error)]);
    printf ("\tdata[%d] = [%p]\n", *(this_ -> whole_size), this_ -> data);
    printf ("\t{\n");
    int itt = 0;
    for (itt = 0; itt < *(this_ -> whole_size); itt++)
    {
        fprintf (output, "\t\t[%d]: %d\t%s\n", itt, *(this_ -> data + itt),
                                               *(this_ -> data + itt) == POISON_CONST ? "[poison]" : "");
        printf ("\t\t[%d]: %d\t%s\n", itt, *(this_ -> data + itt),
                                      *(this_ -> data + itt) == POISON_CONST ? "[poison]" : "");
    }
    fprintf (output, "\t}\n");
    fprintf (output, "\tsize = %d\n", *(this_ -> size_));
    fprintf (output, "}\n");
    printf ("\t}\n");
    printf ("\tsize = %d\n", *(this_ -> size_));
    printf ("}\n");

    fclose (output);
}

int control_sum (void *pointer, int bytes_cnt)
{
    int ans = 1;
    int i = 0;
    for (i = 0; i < bytes_cnt; i++)
    {
        ans *= (int) *((unsigned char *) ((int) pointer) + i);
        ans %= 1000000;
    }
    return ans;
}

void to_mem_view (unsigned int *num_pointer, unsigned char *ans)
{
    int i = 0;
    unsigned char c_now = '\0';
    for (i = 0; i < 4; i++)
    {
        c_now = *((unsigned char *) ((int) num_pointer) + i);
        printf ("%c %d %1d\n", c_now, c_now, *((int *) ((int) num_pointer) + i));
        if (c_now < 30)
        {
            c_now += 30;
        }
        ans[i] = c_now;
    }
    ans[4] = '\0';
}

int is_pointer_wrong (void *pointer)
{
    if (((int) pointer % 4 == 0) && (pointer != NULL))
    {
        return 0;
    }
    return 1;
}

int stack_ok (struct stack_t *this_)
{
    if (is_pointer_wrong (this_))
    {
        *(this_ -> error) = wrong_stack_pointer;
        dump (this_);
        return 0;
    }
    if (is_pointer_wrong (this_ -> all_struc_data))
    {
        *(this_ -> error) = wrong_all_struc_data_pointer;
        dump (this_);
        return 0;
    }
    if (this_ -> buf_canary_front != (unsigned int *) this_ -> all_struc_data)
    {
        *(this_ -> error) = wrong_buf_canary_front_pointer;
        dump (this_);
        return 0;
    }
    if (this_ -> all_struc_data_size != (int *) this_ -> all_struc_data + 1)
    {
        *(this_ -> error) = wrong_all_struc_data_size_pointer;
        dump (this_);
        return 0;
    }
    if (this_ -> size_ != (int *) this_ -> all_struc_data + 2)
    {
        *(this_ -> error) = wrong_size__pointer;
        dump (this_);
        return 0;
    }
    if (this_ -> univ_itt != (int *) this_ -> all_struc_data + 3)
    {
        *(this_ -> error) = wrong_univ_itt_pointer;
        dump (this_);
        return 0;
    }
    if (this_ -> error != (int *) this_ -> all_struc_data + 4)
    {
        *(this_ -> error) = wrong_error_pointer;
        dump (this_);
        return 0;
    }
    if (this_ -> whole_size != (int *) this_ -> all_struc_data + 5)
    {
        *(this_ -> error) = wrong_whole_size_pointer;
        dump (this_);
        return 0;
    }
    if (this_ -> name != (char *) this_ -> all_struc_data + 24)
    {
        *(this_ -> error) = wrong_name_pointer;
        dump (this_);
        return 0;
    }

    int size_of_name = (int) this_ -> data - (int) this_ -> name;

    if (this_ -> data != (elem_t *) this_ -> all_struc_data +
        (int) ceil ((float) 24 / sizeof (elem_t)) +
        (int) ceil ((float) size_of_name / sizeof (elem_t)))
    {
        *(this_ -> error) = wrong_data_pointer;
        dump (this_);
        return 0;
    }
    if (this_ -> buf_canary_end != (unsigned int *) this_ -> all_struc_data +
        (int) ceil ((float) 24 / sizeof (elem_t)) +
        (int) ceil ((float) size_of_name / sizeof (elem_t)) +
        (int) ceil ((float) (*(this_ -> whole_size) * sizeof (elem_t)) / sizeof (unsigned int)))
    {
        *(this_ -> error) = wrong_buf_canary_end_pointer;
        dump (this_);
        return 0;
    }
    if (*(this_ -> error) == stack_was_destructed)
    {
        dump (this_);
        return 0;
    }
    if (*(this_ -> size_) > *(this_ -> whole_size))
    {
        *(this_ -> error) = stack_is_overflowed;
        dump (this_);
        return 0;
    }
    if (*(this_ -> size_) < 0)
    {
        *(this_ -> error) = stack_is_empty;
        dump (this_);
        return 0;
    }
    if (*(this_ -> univ_itt) != 0)
    {
        *(this_ -> error) = univ_itt_is_not_0;
        dump (this_);
        return 0;
    }
    if (this_ -> canary_front != CANARY_FRONT)
    {
        *(this_ -> error) = broken_front_struct_canary;
        dump (this_);
        return 0;
    }
    if (this_ -> canary_end != CANARY_END)
    {
        *(this_ -> error) = broken_end_struct_canary;
        dump (this_);
        return 0;
    }
    if (*(this_ -> buf_canary_front) != BUF_CANARY_FRONT)
    {
        *(this_ -> error) = broken_front_buf_canary;
        dump (this_);
        return 0;
    }
    if (*(this_ -> buf_canary_end) != BUF_CANARY_END)
    {
        *(this_ -> error) = broken_end_buf_canary;
        dump (this_);
        return 0;
    }
    if (this_ -> cntrl_sum = control_sum (this_ -> all_struc_data, *(this_ -> all_struc_data_size)) *
                             control_sum ((int *) &this_ + 2, 13 * 4) % 1000000)   //!!!
    {
        *(this_ -> error) = broken_cntrl_sum;
        dump (this_);
        return 0;
    }
    if (*(this_ -> error) != no_errors)
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

    if (*(this_ -> size_) >= *(this_ -> whole_size))
    {
        stack_resize_while (this_, *(this_ -> all_struc_data_size) + sizeof (elem_t), 1);
        if (*(this_ -> size_) >= *(this_ -> whole_size))
        {
            *(this_ -> error) = stack_is_overflowed;
            return 1;
        }
    }
    *(this_ -> data + *(this_ -> size_)) = value;
    (*(this_ -> size_))++;
    this_ -> cntrl_sum = control_sum (this_ -> all_struc_data, *(this_ -> all_struc_data_size)) *
                         control_sum ((int *) &this_ + 2, 13 * 4) % 1000000;  //!!!

    assert (stack_ok (this_));

    return 0;
}

int stack_pop (struct stack_t *this_)
{
    assert (stack_ok (this_));

    (*(this_ -> size_))--;
    if (*(this_ -> size_) < 0)
    {
        *(this_ -> error) = stack_is_empty;
        return POISON_CONST;
    }
    int ans = *(this_ -> data + *(this_ -> size_));
    *(this_ -> data + *(this_ -> size_)) = POISON_CONST;
    this_ -> cntrl_sum = control_sum (this_ -> all_struc_data, *(this_ -> all_struc_data_size)) *
                         control_sum ((int *) &this_ + 2, 13 * 4) % 1000000;  //!!!

    assert (stack_ok (this_));

    return ans;
}

int stack_size (struct stack_t *this_)
{
    assert (stack_ok (this_));

    return *(this_ -> size_);
}

int stack_resize (struct stack_t *this_, int not_constr)
{
    if (not_constr) assert (stack_ok (this_));

    int size_of_name = (int) this_ -> data - (int) this_ -> name;
    this_ -> all_struc_data = (void *) realloc (this_ -> all_struc_data, (size_t) *(this_ -> all_struc_data_size) + CHUNK);

    this_ -> buf_canary_front = (unsigned int *) this_ -> all_struc_data;
    this_ -> all_struc_data_size = (int *) this_ -> all_struc_data + 1;
    this_ -> size_               = (int *) this_ -> all_struc_data + 2;
    this_ -> univ_itt            = (int *) this_ -> all_struc_data + 3;
    this_ -> error               = (int *) this_ -> all_struc_data + 4;
    this_ -> whole_size          = (int *) this_ -> all_struc_data + 5;
    this_ -> name                = (char *) this_ -> all_struc_data + 24;
    this_ -> data                = (elem_t *) this_ -> all_struc_data + (int) ceil ((float) 24 / sizeof (elem_t)) +
                                   (int) ceil ((float) size_of_name / sizeof (elem_t));

    *(this_ -> all_struc_data_size) += CHUNK;
    *(this_ -> whole_size) += (int) floor ((float) (CHUNK) / sizeof (elem_t));
    int itt = 0;
    for (itt = *(this_ -> size_); itt < *(this_ -> whole_size); itt++)
    {
        *(this_ -> data + itt) = POISON_CONST;
    }

    this_ -> buf_canary_end = (unsigned int *) this_ -> all_struc_data + (int) ceil ((float) 24 / sizeof (elem_t)) +
                              (int) ceil ((float) size_of_name / sizeof (elem_t)) +
                              (int) ceil ((float) (*(this_ -> whole_size) * sizeof (elem_t)) / sizeof (unsigned int));

    *(this_ -> buf_canary_end) = BUF_CANARY_END;
    this_ -> cntrl_sum = control_sum (this_ -> all_struc_data, *(this_ -> all_struc_data_size)) *
                         control_sum ((int *) &this_ + 2, 13 * 4) % 1000000;  //!!!

    if (not_constr) assert (stack_ok (this_));

    return 0;
}

int stack_resize_while (struct stack_t *this_, int size_needed, int not_constr)
{
    if (not_constr) assert (stack_ok (this_));

    while (*(this_ -> all_struc_data_size) < size_needed)
    {
        stack_resize (this_, 0);
    }

    if (not_constr) assert (stack_ok (this_));

    return 0;
}

int stack_destruct (struct stack_t *this_)
{
    assert (stack_ok (this_));

    free (this_ -> all_struc_data);
    *(this_ -> error) = stack_was_destructed;
    return 0;
}

int main ()
{
    struct stack_t stk1 = {};
    stack_construct (stk1);
    int size_of_name = (int) stk1.data - (int) stk1.name;
    for (int i = 0; i < 10; i++)
    {
        stack_push (&stk1, 10);
    }
    int val = 0;
    //stk1.all_struc_data = (void *) 1;
    //stk1.canary_end = 1;
    //*stk1.buf_canary_front = 1;
    //stk1.buf_canary_front = (unsigned int *) 1;

    //*(stk1.buf_canary_end) = 1094861636;

    for (int i = 0; i < 10; i++)
    {
        val = stack_pop (&stk1);
    }
    stack_destruct (&stk1);
    printf ("%d\n", val);
    return 0;
}

