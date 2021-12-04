/**
 * vector
 * CS 241 - Spring 2021
 */
#include "sstring.h"

int main(int argc, char *argv[]) {
    // TODO create some tests
    sstring * str = cstr_to_sstring("abc{}, ");
    sstring * strb = cstr_to_sstring("def{}");
    sstring_append(str, strb);
    char * cstr = sstring_to_cstr(str);
    printf("%s\n", cstr);
    char *slice = sstring_slice(str, 0, 0);
    char *slice2 = sstring_slice(str, 1, 4);
    printf("%s\n", slice);
    printf("%s\n", slice2);
    sstring_substitute(str, 0, "0","ohhh");
    char * cstr1 = sstring_to_cstr(str);
    printf("%s\n", cstr1);
    sstring_split(str, '}');
    return 0;
}
