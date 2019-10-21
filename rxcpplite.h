#if !defined(__h_rxcpplite__)
#define __h_rxcpplite__

#include <exception>
#include <functional>

namespace rxcpplite
{
namespace cxx   = std;
using error_ptr = std::exception_ptr;


using value = int;


using fn_next_t     = std::function<void(value)>;
using fn_error_t    = std::function<void(error_ptr)>;
using fn_complete_t = std::function<void()>;


class subscriber : public cxx::enable_shared_from_this<subscriber>
{
public:
    using sp = cxx::shared_ptr<subscriber>;
    
private:
    fn_next_t       m_fnNext;
    fn_error_t      m_fnError;
    fn_complete_t   m_fnComplete;
    
    void setup(
        fn_next_t     next     = fn_next_t(),
        fn_error_t    error    = fn_error_t(),
        fn_complete_t complete = fn_complete_t()
    )
    {
        m_fnNext     = next;
        m_fnError    = error;
        m_fnComplete = complete;
    }
    
protected:
    
public:
    virtual ~subscriber() = default;
    
    static sp create(
        fn_next_t     next     = fn_next_t(),
        fn_error_t    error    = fn_error_t(),
        fn_complete_t complete = fn_complete_t()
    )
    {
        auto s = cxx::make_shared<subscriber>();
        s->setup(next, error, complete);
        return s;
    }
    
    void next(value v)
    {
        if(m_fnNext){
            try{
                m_fnNext(v);
            }
            catch(...){
                error(cxx::current_exception());
            }
        }
    }
    
    void error(error_ptr err)
    {
        auto fnError = m_fnError;
        m_fnNext = fn_next_t();
        m_fnError = fn_error_t();
        m_fnComplete = fn_complete_t();
        if(fnError){
            fnError(err);
        }
    }
    
    void complete()
    {
        try{
            auto fnComplete = m_fnComplete;
            m_fnNext = fn_next_t();
            m_fnComplete = fn_complete_t();
            if(fnComplete){
                fnComplete();
            }
        }
        catch(...){
            error(cxx::current_exception());
        }
    }
};

class observable : public cxx::enable_shared_from_this<observable>
{
private:
    using sp = cxx::shared_ptr<observable>;
    
    
private:
    cxx::function<void(subscriber::sp)>    m_generator;
    
protected:
    
    
public:
    static sp create(cxx::function<void(subscriber::sp)> generator)
    {
        auto s = cxx::make_shared<observable>();
        s->m_generator = generator;
        return s;
    }
    
    virtual ~observable() = default;
    
    void subscribe(
        fn_next_t     next     = fn_next_t(),
        fn_error_t    error    = fn_error_t(),
        fn_complete_t complete = fn_complete_t()
    )
    {
        auto _THIS = shared_from_this();
        m_generator(subscriber::create([_THIS, next](value v){
            next(v);
        }, [_THIS, error](error_ptr e){
            error(e);
        }, [_THIS, complete](){
            complete();
        }));
    }
    
    observable::sp tap(
        fn_next_t     next     = fn_next_t(),
        fn_error_t    error    = fn_error_t(),
        fn_complete_t complete = fn_complete_t()
    )
    {
        auto _THIS = shared_from_this();
        return observable::create([_THIS, next, error, complete](subscriber::sp s){
            _THIS->subscribe([_THIS, next, s](value v){
                if(next){
                    next(v);
                }
                s->next(v);
            }, [_THIS, error, s](error_ptr e){
                if(error){
                    error(e);
                }
                s->error(e);
            }, [_THIS, complete, s](){
                if(complete){
                    complete();
                }
                s->complete();
            });
        });
    }
    
    observable::sp map(cxx::function<value(value)> f)
    {
        auto _THIS = shared_from_this();
        return observable::create([_THIS, f](subscriber::sp s){
            _THIS->subscribe([_THIS, f, s](value v){
                s->next(f(v));
            }, [_THIS, s](error_ptr e){
                s->error(e);
            }, [_THIS, s](){
                s->complete();
            });
        });
    }
    
    observable::sp flat_map(cxx::function<observable::sp(value)> f)
    {
        auto _THIS = shared_from_this();
        return observable::create([_THIS, f](subscriber::sp s){
            _THIS->subscribe([_THIS, f, s](value v){
                f(v)->subscribe([_THIS, s](value fv){
                    s->next(fv);
                }, [_THIS, s](error_ptr fe){
                    s->error(fe);
                }, [_THIS, s](){
                    s->complete();
                });
            }, [_THIS, s](error_ptr e){
                s->error(e);
            }, [_THIS, s](){
                s->complete();
            });
        });
    }
};
    
} /* rxcpplite */

#endif /* !defined(__h_rxcpplite__) */
