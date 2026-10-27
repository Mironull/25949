#include <stdio.h>
#include <time.h>

int main() {
    time_t t;
    struct tm *tm_ptr;
    time(&t);
    tm_ptr = gmtime(&t);
    tm_ptr->tm_hour -= 8;
    if (tm_ptr->tm_hour < 0) {
        tm_ptr->tm_hour += 24;
        tm_ptr->tm_mday--;
    }
    printf("California time: %s", asctime(tm_ptr));
    
    return 0;
}
