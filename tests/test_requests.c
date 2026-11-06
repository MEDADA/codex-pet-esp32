#include <assert.h>
#include <stdio.h>
#include "buddy_requests.h"
int main(void){
    buddy_request_t r={0};uint8_t out[14];
    uint8_t frame[]={0xe9,1,1,0,0,0,5,0,0,0,9,0,10,1,0,0,0,0,5,1};
    assert(buddy_request_receive(&r,frame,sizeof(frame),100));
    r.verified=true;
    assert(buddy_request_encode(&r,1,9,200,out)==14);
    assert(out[0]==0xea&&out[5]==1&&out[9]==5&&out[13]==9);
    assert(!buddy_request_encode(&r,1,9,201,out));
    uint8_t ack[]={0xeb,1,0,1,0};assert(buddy_request_ack(&r,ack,5));
    assert(r.result==1);
    buddy_request_clear(&r);assert(r.result==1);
    assert(!buddy_request_encode(&r,1,9,10200,out));
    frame[10]=10;assert(buddy_request_receive(&r,frame,20,300));
    assert(!buddy_request_encode(&r,1,9,400,out));
    assert(!buddy_request_receive(&r,frame,7,400));
    buddy_request_clear(&r);assert(!buddy_request_encode(&r,1,10,500,out));
    assert(buddy_request_receive(&r,frame,20,600));r.verified=true;
    assert(buddy_request_encode(&r,1,10,700,out)==14);
    assert(buddy_request_receive(&r,frame,20,15000));
    buddy_request_check_timeout(&r,15700);assert(!r.pending&&r.result==3&&r.result_request==10);
    puts("PASS bound request expiry, stale detail, pending duplicate, matching acknowledgement");
}
