#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

class noncopyable {
protected:
    /**
     * Disallow construction of NonCopyable objects from outside of its hierarchy.
     */
    noncopyable() = default;
    /**
     * Disallow destruction of NonCopyable objects from outside of its hierarchy.
     */
    ~noncopyable() = default;

public:
    /**
     * Define copy constructor as deleted. Any attempt to copy construct
     * a NonCopyable will fail at compile time.
     */
    noncopyable(const noncopyable &) = delete;
    /**
     * Define copy assignment operator as deleted. Any attempt to copy assign
     * a NonCopyable will fail at compile time.
     */
    noncopyable& operator=(const noncopyable &) = delete;
};


#endif