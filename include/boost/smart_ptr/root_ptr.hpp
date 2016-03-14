/**
    @file
    Boost node_ptr.hpp header file.

    @author
    Copyright (c) 2008-2016 Phil Bouchard <pbouchard8@gmail.com>.

    @note
    Distributed under the Boost Software License, Version 1.0.

    See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt

    See http://www.boost.org/libs/smart_ptr/doc/index.html for documentation.


    Thanks to: Steven Watanabe <watanabesj@gmail.com>
*/


#ifndef BOOST_NODE_PTR_INCLUDED
#define BOOST_NODE_PTR_INCLUDED


#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4355 )

#include <new.h>
#endif

#ifndef BOOST_DISABLE_THREADS
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#endif

#include <boost/smart_ptr/detail/intrusive_list.hpp>
#include <boost/smart_ptr/detail/intrusive_stack.hpp>
#include <boost/smart_ptr/detail/classof.hpp>
#include <boost/smart_ptr/detail/node_ptr_base.hpp>


namespace boost
{

namespace smart_ptr
{

namespace detail
{


struct node_base;


} // namespace detail

} // namespace smart_ptr


/**
    Set header.
    
    Proxy object used to count the number of pointers from the stack are referencing pointee objects belonging to the same @c node_proxy .
*/

class node_proxy
{
    template <typename> friend class node_ptr;
    template <typename> friend class root_ptr;

    bool destroying_;                                   /**< Destruction sequence initiated. */
    smart_ptr::detail::intrusive_list node_list_;                     /**< List of all pointee objects belonging to a @c node_proxy . */

#ifndef BOOST_DISABLE_THREADS
    static mutex & static_mutex()                   /**< Main global mutex used for thread safety */
    {
        static mutex mutex_;
        
        return mutex_;
    }
#endif

    /**
        Initialization of a single @c node_proxy .
    */
    
    node_proxy() : destroying_(false)
    {
    }
    
    
    ~node_proxy()
    {
        reset();
    }
    
    
    bool destroying() const
    {
        return destroying_;
    }

    
    void destroying(bool b)
    {
        destroying_ = b;
    }

    
    /**
        Enlist & initialize pointee objects belonging to the same @c node_proxy .  This initialization occurs when a pointee object is affected to the first pointer living on the stack it encounters.
        
        @param  p   Pointee object to initialize.
    */
    
    void init(smart_ptr::detail::node_base * p)
    {
        node_list_.push_back(& p->node_tag_);
    }
    
    
    void reset()
    {
        using namespace smart_ptr::detail;
        
        destroying(true);

        for (intrusive_list::iterator<node_base, &node_base::node_tag_> m = node_list_.begin(), n = node_list_.begin(); m != node_list_.end(); m = n)
        {
            ++ n;
            delete &* m;
        }
        
        destroying(false);
    }
};


#define TEMPLATE_DECL(z, n, text) BOOST_PP_COMMA_IF(n) typename T ## n
#define ARGUMENT_DECL(z, n, text) BOOST_PP_COMMA_IF(n) T ## n const & t ## n
#define PARAMETER_DECL(z, n, text) BOOST_PP_COMMA_IF(n) t ## n

#define BEFRIEND_MAKE_NODE(z, n, text)																			    	\
    template <typename V, BOOST_PP_REPEAT(n, TEMPLATE_DECL, 0)>										                    \
        friend node_ptr<V> text(BOOST_PP_REPEAT(n, ARGUMENT_DECL, 0));

#define CONSTRUCT_MAKE_NODE1(z, n, text)																			    \
    template <typename V, BOOST_PP_REPEAT(n, TEMPLATE_DECL, 0), typename PoolAllocator = pool_allocator<V> >										                    \
        node_ptr<V> text(BOOST_PP_REPEAT(n, ARGUMENT_DECL, 0))															\
        {																												\
            return node_ptr<V>(new node<V, PoolAllocator>(BOOST_PP_REPEAT(n, PARAMETER_DECL, 0)));									\
        }

#define CONSTRUCT_MAKE_NODE2(z, n, text)                                                                                \
    template <typename V, BOOST_PP_REPEAT(n, TEMPLATE_DECL, 0), typename PoolAllocator = pool_allocator<V> >                                                          \
        node_ptr<V> text(smart_ptr::detail::node_ptr_base<smart_ptr::detail::node_proxy> & q, BOOST_PP_REPEAT(n, ARGUMENT_DECL, 0))                                                           \
        {                                                                                                               \
            return node_ptr<V>(q, new node<V, PoolAllocator>(BOOST_PP_REPEAT(n, PARAMETER_DECL, 0)));                                   \
        }

#define CONSTRUCT_MAKE_NODE3(z, n, text)                                                                               \
    template <typename V, BOOST_PP_REPEAT(n, TEMPLATE_DECL, 0), typename PoolAllocator = pool_allocator<V> >                                                          \
        node_ptr<V> text(BOOST_PP_REPEAT(n, ARGUMENT_DECL, 0))                                                           \
        {                                                                                                               \
            return node_ptr<V>(new fastnode<V>(BOOST_PP_REPEAT(n, PARAMETER_DECL, 0)));                                   \
        }

#define CONSTRUCT_MAKE_NODE4(z, n, text)                                                                                \
    template <typename V, BOOST_PP_REPEAT(n, TEMPLATE_DECL, 0), typename PoolAllocator = pool_allocator<V> >                                                          \
        node_ptr<V> text(smart_ptr::detail::node_ptr_base<smart_ptr::detail::node_proxy> & q, BOOST_PP_REPEAT(n, ARGUMENT_DECL, 0))                                                           \
        {                                                                                                               \
            return node_ptr<V>(q, new fastnode<V>(BOOST_PP_REPEAT(n, PARAMETER_DECL, 0)));                                   \
        }


/**
    Deterministic region based memory manager.
    
    Complete memory management utility on top of standard reference counting.
*/

template <typename T>
    class node_ptr : public smart_ptr::detail::node_ptr_base<T>
    {
        template <typename> friend class node_ptr;

        typedef smart_ptr::detail::node_ptr_base<T> base;
        
        using base::share;
        using base::po_;

        node_proxy const & x_;                      /**< Pointer to the @c node_proxy node @c node_ptr<> belongs to. */
        
    public:
        using base::reset;

        /**
            Initialization of a pointer.
            
            @param  p   New pointee object to manage.
        */
        
        explicit node_ptr(node_proxy const & x) : base(), x_(x)
        {
        }

            
        /**
            Initialization of a pointer.
            
            @param	p	New pointee object to manage.
        */
        
        template <typename V, typename PoolAllocator>
            explicit node_ptr(node_proxy const & x, node<V, PoolAllocator> * p) : base(p), x_(x)
            {
                const_cast<node_proxy &>(x_).init(p);
            }

        
    public:
        typedef T                           value_type;


        /**
            Initialization of a pointer.
            
            @param	p	New pointer to manage.
        */

        template <typename V>
            node_ptr(node_ptr<V> const & p) : base(p), x_(p.x_)
            {
#ifndef BOOST_DISABLE_THREADS
                mutex::scoped_lock scoped_lock(node_proxy::static_mutex());
#endif
            }

        
        /**
            Initialization of a pointer.
            
            @param	p	New pointer to manage.
        */

            node_ptr(node_ptr<T> const & p) : base(p), x_(p.x_)
            {
#ifndef BOOST_DISABLE_THREADS
                mutex::scoped_lock scoped_lock(node_proxy::static_mutex());
#endif
            }


        /**
            Assignment.
            
            @param	p	New pointer to manage.
        */
            
        template <typename V>
            node_ptr & operator = (node_ptr<V> const & p)
            {
#ifndef BOOST_DISABLE_THREADS
                mutex::scoped_lock scoped_lock(node_proxy::static_mutex());
#endif

                base::operator = (p);

                return * this;
            }


        /**
            Assignment.
            
            @param	p	New pointer to manage.
        */

        node_ptr & operator = (node_ptr<T> const & p)
        {
            return operator = <T>(p);
        }

        /**
            Assignment.
            
            @param  p   New pointer to manage.
        */

        template <typename V, typename PoolAllocator>
            node_ptr & operator = (node<V, PoolAllocator> * p)
            {
                const_cast<node_proxy &>(x_).init(p);

                base::operator = (p);
                
                return * this;
            }

        template <typename V>
            void reset(node_ptr<V> const & p)
            {
                operator = <T>(p);
            }
        
        template <typename V, typename PoolAllocator>
            void reset(node<V, PoolAllocator> * p)
            {
                operator = <T>(p);
            }
        
        bool cyclic() const
        {
            return x_.destroying();
        }

        ~node_ptr()
        {
            if (cyclic())
                base::po_ = 0;
        }

#if 0 //defined(BOOST_HAS_RVALUE_REFS)
    public:
        node_ptr(node_ptr<T> && p): base(p.po_), x_(p.x_)
        {
            p.po_ = 0;
        }

        template<class Y>
            node_ptr(node_ptr<Y> && p): base(p.po_), x_(p.x_)
            {
                p.po_ = 0;
            }

        node_ptr<T> & operator = (node_ptr<T> && p)
        {
            std::swap(po_, p.po_);
            std::swap(x_, p.x_);
            
            return *this;
        }

        template<class Y>
            node_ptr & operator = (node_ptr<Y> && p)
            {
                std::swap(po_, p.po_);
                std::swap(x_, p.x_);
                
                return *this;
            }
#endif
    };


/**
    Helper.
*/
    
template <typename T>
    struct root_ptr : node_proxy, node_ptr<T>
    {
        using node_ptr<T>::reset;
        
        root_ptr() : node_proxy(), node_ptr<T>(* static_cast<node_proxy *>(this))
        {
        }
        
        
        root_ptr(root_ptr const & p) : node_proxy(const_cast<root_ptr &>(p)), node_ptr<T>(* static_cast<node_proxy *>(this))
        {
        }
        
        
        /**
            Initialization of a pointer.
            
            @param  p   New pointee object to manage.
        */
        
        template <typename V, typename PoolAllocator>
            root_ptr(node<V, PoolAllocator> * p) : node_ptr<T>(*this, p)
            {
            }
            
            
        node_ptr<T> & operator = (node_ptr<T> const & p)
        {
            return node_ptr<T>::operator = (p);
        }

        template <typename V, typename PoolAllocator>
            node_ptr<T> & operator = (node<V, PoolAllocator> * p)
            {
                return node_ptr<T>::operator = (p);
            }
    };


template <typename V, typename PoolAllocator = pool_allocator<V> >
    node_ptr<V> make_node()
    {
        return node_ptr<V>(new node<V, PoolAllocator>());
    }

template <typename V, typename PoolAllocator = pool_allocator<V> >
    node_ptr<V> make_node(smart_ptr::detail::node_ptr_base<smart_ptr::detail::node_proxy> & q)
    {
        return node_ptr<V>(q, new node<V, PoolAllocator>());
    }

template <typename V, typename PoolAllocator = pool_allocator<V> >
    node_ptr<V> make_fastnode()
    {
        return node_ptr<V>(new fastnode<V>());
    }

template <typename V, typename PoolAllocator = pool_allocator<V> >
    node_ptr<V> make_fastnode(smart_ptr::detail::node_ptr_base<smart_ptr::detail::node_proxy> & q)
    {
        return node_ptr<V>(q, new fastnode<V>());
    }

template <typename T>
    bool operator == (node_ptr<T> const &a1, node_ptr<T> const &a2)
    {
        return a1.get() == a2.get();
    }

template <typename T>
    bool operator != (node_ptr<T> const &a1, node_ptr<T> const &a2)
    {
        return a1.get() != a2.get();
    }


BOOST_PP_REPEAT_FROM_TO(1, 10, CONSTRUCT_MAKE_NODE1, make_node)
BOOST_PP_REPEAT_FROM_TO(1, 10, CONSTRUCT_MAKE_NODE2, make_node)
BOOST_PP_REPEAT_FROM_TO(1, 10, CONSTRUCT_MAKE_NODE3, make_fastnode)
BOOST_PP_REPEAT_FROM_TO(1, 10, CONSTRUCT_MAKE_NODE4, make_fastnode)

} // namespace boost


#if defined(_MSC_VER)
#pragma warning( pop )
#endif


#endif // #ifndef BOOST_NODE_PTR_INCLUDED