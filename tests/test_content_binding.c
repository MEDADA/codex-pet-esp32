#include <assert.h>
#include <stdio.h>
#include "buddy_requests.h"
int main(void){
    buddy_request_t r={0};uint8_t frame[20]={0xe9,1,1,0,0,0,5,0,0,0,9,0,15,1,0,0,0,0,5,1};
    assert(buddy_request_receive(&r,frame,sizeof(frame),0));
    r.digest=buddy_request_fingerprint(1,3,"Shell","project","model","build");
    assert(buddy_request_content_matches(&r,1,3,"Shell","project","model","build"));
    assert(!buddy_request_content_matches(&r,1,3,"Shell","project","model","old!"));
    assert(!buddy_request_content_matches(&r,1,3,"Shell","wrong","model","build"));
    r.complete=false;assert(!buddy_request_content_matches(&r,1,3,"Shell","project","model","build"));
    puts("PASS actionable context requires complete matching displayed content");
}
