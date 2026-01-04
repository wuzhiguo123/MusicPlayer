#include <stdio.h>
#include <json-c/json.h>

int main() {
    json_object* jobj = json_object_new_object();
    if (jobj) {
        printf("json-c test successful!\n");
        json_object_put(jobj);
        return 0;
    }
    return 1;
}