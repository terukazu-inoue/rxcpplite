#include "rxcpplite.h"
#include <iostream>
#include <sstream>

class Test
{
public:
    auto& log() { return std::cout; }
    
    Test()
    {
        using namespace rxcpplite;
        
        observable::create([=](subscriber::sp s){
            s->next(typed_value<int>::create(1));
            s->complete();
        })
        ->tap([=](abstruct_value::sp v){
            log() << "tap(0):next " << v->value<int>() << std::endl;
        }, [=](error_ptr e){
            log() << "tap(0):error" << std::endl;
        }, [=](){
            log() << "tap(0):complete" << std::endl;
        })
        ->map([=](abstruct_value::sp v){
            return typed_value<int>::create(v->value<int>() + 1);
        })
        ->tap([=](abstruct_value::sp v){
            log() << "tap(1):next " << v->value<int>() << std::endl;
        }, [=](error_ptr e){
            log() << "tap(1):error" << std::endl;
        }, [=](){
            log() << "tap(1):complete" << std::endl;
        })
        ->map([=](abstruct_value::sp v){
            return typed_value<int>::create(v->value<int>() * 2);
        })
        ->tap([=](abstruct_value::sp v){
            log() << "tap(2):next " << v->value<int>() << std::endl;
        }, [=](error_ptr e){
            log() << "tap(2):error" << std::endl;
        }, [=](){
            log() << "tap(2):complete" << std::endl;
        })
        ->flat_map([=](abstruct_value::sp v){
            return observable::create([=](subscriber::sp s){
                std::stringstream ss;
                ss << "value = " << v->value<int>();
                s->next(typed_value<std::string>::create(ss.str()));
                s->next(typed_value<std::string>::create("ABC"));
                s->next(typed_value<std::string>::create("DEF"));
                s->complete();
            });
        })
        ->tap([=](abstruct_value::sp v){
            log() << "tap(3):next " << v->value<std::string>() << std::endl;
        }, [=](error_ptr e){
            log() << "tap(3):error" << std::endl;
        }, [=](){
            log() << "tap(3):complete" << std::endl;
        })
        ->subscribe([=](abstruct_value::sp v){
            log() << "subscribe:next " << v->value<std::string>() << std::endl;
        }, [=](error_ptr){
            log() << "subscribe:error" << std::endl;
        }, [=](){
            log() << "subscribe:complete" << std::endl;
        });
    }
};

auto t = new Test();
