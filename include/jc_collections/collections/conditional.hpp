#ifndef COND_H
#define COND_H

enum class TempOption
{
    first,
    second,
    third
};

template <TempOption opt, typename X, typename Y, typename Z>
struct cond;

template <typename X, typename Y, typename Z>
struct cond<TempOption::first, X, Y, Z>
{
    using type = X;
};

template <typename X, typename Y, typename Z>
struct cond<TempOption::second, X, Y, Z>
{
    using type = Y;
};

template <typename X, typename Y, typename Z>
struct cond<TempOption::third, X, Y, Z>
{
    using type = Z;
};

#endif
