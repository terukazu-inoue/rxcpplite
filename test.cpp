#include "rxcpplite.h"
#include <iostream>

class Test
{
public:
    auto& log() { return std::cout; }
    
    Test()
    {
        using namespace rxcpplite;
        
        observable::create([=](subscriber::sp s){
            s->next(1);
            s->complete();
        })
        ->tap([=](value v){
            log() << "tap(0):next " << v << std::endl;
        }, [=](error_ptr e){
            log() << "tap(0):error" << std::endl;
        }, [=](){
            log() << "tap(0):complete" << std::endl;
        })
        ->map([=](value v){
            return v + 1;
        })
        ->tap([=](value v){
            log() << "tap(1):next " << v << std::endl;
        }, [=](error_ptr e){
            log() << "tap(1):error" << std::endl;
        }, [=](){
            log() << "tap(1):complete" << std::endl;
        })
        ->map([=](value v){
            return v * 2;
        })
        ->tap([=](value v){
            log() << "tap(2):next " << v << std::endl;
        }, [=](error_ptr e){
            log() << "tap(2):error" << std::endl;
        }, [=](){
            log() << "tap(2):complete" << std::endl;
        })
        ->flat_map([=](value v){
            return observable::create([=](subscriber::sp s){
                s->next(v * 2);
                s->next(v * 2 + 1);
                s->complete();
            });
        })
        ->tap([=](value v){
            log() << "tap(3):next " << v << std::endl;
        }, [=](error_ptr e){
            log() << "tap(3):error" << std::endl;
        }, [=](){
            log() << "tap(3):complete" << std::endl;
        })
        ->subscribe([=](value v){
            log() << "subscribe:next " << v << std::endl;
        }, [=](error_ptr){
            log() << "subscribe:error" << std::endl;
        }, [=](){
            log() << "subscribe:complete" << std::endl;
        });
    }
};

auto t = new Test();
