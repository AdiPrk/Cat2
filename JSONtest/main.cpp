#include <stdio.h>                                 // include standard I/O library

int main(void) {                                  // program entry point
    printf("Speed Machine Calculator!\n\n");      // header

    /* --- constants --- */
    const double SECONDS_PER_100_MILES = 1609.35; // given directly in the problem
    const double MILES_IN_100 = 100.0;            // 100 miles distance
    const double LIGHTYEAR_MILES = 6.000e12;      // assignment assumption
    const double SECS_PER_HOUR = 3600.0;          // conversion: seconds Å® hours
    const double SECS_PER_MIN = 60.0;             // conversion: seconds Å® minutes
    const double HOURS_PER_DAY = 24.0;            // hours per day
    const double DAYS_PER_YEAR = 365.25;          // days per year (tweak if needed)

    /* --- derived values --- */
    double seconds_per_mile = SECONDS_PER_100_MILES / MILES_IN_100; // seconds per 1 mile
    double mph = SECS_PER_HOUR / seconds_per_mile;                  // miles per hour

    /* times for 100 miles */
    double hours_for_100 = SECONDS_PER_100_MILES / SECS_PER_HOUR;   // hours
    double minutes_for_100 = SECONDS_PER_100_MILES / SECS_PER_MIN;  // minutes
    double seconds_for_100 = SECONDS_PER_100_MILES;                 // seconds

    /* time for 1 lightyear in years */
    double years_for_lightyear = LIGHTYEAR_MILES / (mph * 24 * 365.24);

    /* --- output section --- */
    printf("Speed in miles per hour is: %.2fmph\n\n", mph);

    printf("Given this speed...\n");
    printf("It will take %.2f hours to go 100 miles\n", hours_for_100);
    printf("It will take %.2f minutes to go 100 miles\n", minutes_for_100);
    printf("It will take %.2f seconds to go 100 miles\n", seconds_for_100);
    printf("It will take %.2f years to go 1 lightyear\n", years_for_lightyear);

    return 0;                                     // end program
}
