void next(void){
    if(end_buffer == BUFFERLEN-1){
        end_buffer = 0;
    }else{
        end_buffer++;
    }
}
