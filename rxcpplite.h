#if !defined(__h_rxcpplite__)
#define __h_rxcpplite__

#include <exception>
#include <functional>

namespace rxcpplite
{
namespace cxx   = std;
using error_ptr = std::exception_ptr;

template <typename T> class typed_value;

class abstruct_value : public cxx::enable_shared_from_this<abstruct_value>
{
public:
    using sp = cxx::shared_ptr<abstruct_value>;
    
private:
protected:
    abstruct_value() = default;

public:
    virtual ~abstruct_value() = default;

    template<typename T> cxx::shared_ptr<const typed_value<T>> as() const
    {
        return cxx::dynamic_pointer_cast<const typed_value<T>>(shared_from_this());
    }

    template<typename T> const T& value() const
    {
        return as<T>()->value();
    }
};

template <typename T> class typed_value : public abstruct_value
{
public:
    using sp = cxx::shared_ptr<typed_value<T>>;
    
private:
    T m_value;
    typed_value(T&& v) : m_value(v) {}
    typed_value(const T& v) : m_value(v) {}

protected:

public:
    static sp create(T&& v)
    {
        return sp(new typed_value<T>(v));
    }

    static sp create(const T& v)
    {
        return sp(new typed_value<T>(v));
    }


    const T& value() const { return m_value; }
    operator const T& () const { return m_value; }
};

template <typename T> typename typed_value<T>::sp value(T&& v){
    return typed_value<T>::create(v);
}

template <typename T> typename typed_value<T>::sp value(const T& v){
    return typed_value<T>::create(v);
}

using fn_next_t     = std::function<void(abstruct_value::sp)>;
using fn_error_t    = std::function<void(error_ptr)>;
using fn_complete_t = std::function<void()>;
    
class subscriber : public cxx::enable_shared_from_this<subscriber>
{
public:
    using sp = cxx::shared_ptr<subscriber>;
    
private:
    subscriber() = default;
    
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
        auto s = sp(new subscriber());
        s->setup(next, error, complete);
        return s;
    }
    
    void next(abstruct_value::sp v)
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
public:
    using sp = cxx::shared_ptr<observable>;
    
private:
    observable() = default;
    cxx::function<void(subscriber::sp)>    m_generator;
    
protected:
public:
    static sp create(cxx::function<void(subscriber::sp)> generator)
    {
        auto s = sp(new observable());
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
        m_generator(subscriber::create([_THIS, next](abstruct_value::sp v){
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
            _THIS->subscribe([_THIS, next, s](abstruct_value::sp v){
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
    
    observable::sp map(cxx::function<abstruct_value::sp(abstruct_value::sp)> f)
    {
        auto _THIS = shared_from_this();
        return observable::create([_THIS, f](subscriber::sp s){
            _THIS->subscribe([_THIS, f, s](abstruct_value::sp v){
                s->next(f(v));
            }, [_THIS, s](error_ptr e){
                s->error(e);
            }, [_THIS, s](){
                s->complete();
            });
        });
    }
    
    observable::sp flat_map(cxx::function<observable::sp(abstruct_value::sp)> f)
    {
        auto _THIS = shared_from_this();
        return observable::create([_THIS, f](subscriber::sp s){
            _THIS->subscribe([_THIS, f, s](abstruct_value::sp v){
                f(v)->subscribe([_THIS, s](abstruct_value::sp fv){
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
