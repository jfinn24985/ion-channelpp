#ifndef IONCH_IONCH_IMPLEMENTATION_OBSERVABLE_H
#define IONCH_IONCH_IMPLEMENTATION_OBSERVABLE_H

//Observables are a set of classes for creating mean results from repeated
//measurements or observations.
//
// The file contains a series of classes for the collection of data
// samples for which the mean and variance is required.
//
// These classes are split into two main categories: count observables
// and sample observables. For sample observables each sample
// iteration generates a single value the is inserted into the
// observable.  By contrast, count observables are repeatedly
// incremented during a sampling period and the new mean only
// calculated at the end of the period.  The sampling period starts
// when the counting object is inserted into a count_scope object,
// (see class count_scope) and ends when this object goes out of
// scope.
//
// Several aggregate types exist (1D histograms and 2+D matrices).
// Whether to use these aggregates or simple aggregates of the basic
// observable class depends on the type of observable. In general the
// aggregates should be used when they logically represent multiple
// samples from a single distribution.

#include <boost/serialization/vector.hpp>
#include <cstddef>

// manual includes
#include <cmath>
#include <algorithm>
// manual includes
namespace ionch {

namespace implementation {

template<class Float_Type, class Counter_Type>
class base_observable
 {
   public:
    typedef Counter_Type counter_type;

    typedef Float_Type mean_type;

    mean_type mean_;

    mean_type vars_;

    void insert(counter_type count, mean_type next)
    {
      if (1 != count)
      {
        const Float_Type old_m_ (this->mean_);
        this->mean_ += (next - old_m_) / Float_Type(count);
        this->vars_ += (next - old_m_) * (next - this->mean_);
      }
      else
      {
        // first value
        this->mean_ = next;
        this->vars_ = next;
      }
    }

    // The average value of the sampled distribution
    mean_type mean() const
    {
      return mean_; 
    }

    // The sample variance of the sampled distribution
    mean_type variance(counter_type count) const
    {
        return (count > 1 ? (vars_/mean_type(count - 1)) : mean_type(0));
    }

    //The standard deviation of the sampled distribution
    mean_type stddev(counter_type count) const
    {
        return (count > 1 ? std::sqrt(this->variance(count)) : mean_type(0));
    }

    base_observable()
      : mean_ ()
      , vars_ ()
    {}

    base_observable(mean_type next)
      : mean_ (next)
      , vars_ (next)
    {}

    virtual ~base_observable()
    {}

    base_observable(const base_observable<Float_Type, Counter_Type> & source)
      : mean_(source.mean_)
      , vars_(source.vars_)
    {}

    base_observable<Float_Type, Counter_Type> & operator=(base_observable<Float_Type, Counter_Type> source)
    {
      this->swap (source);
      return *this;
    }

    void swap(const base_observable<Float_Type, Counter_Type> & source)
    {
      std::swap (mean_, source.mean_);
      std::swap (vars_, source.vars_);
    }


};
// Class: observable
//
// Manage a value that is derived from the average of repeated
// observations.
//
// This class uses a successive addition algorithm to calculate the
// mean and variance of a sample value. This is reported to be
// numerically more stable than the traditional sum and sum of
// squares methods.
//
// The default Counter_Type is size_t.  Generally this should be an
// unsigned integer of approximately the same size as the Float_Type
template<class Float_Type, class Counter_Type = std::size_t>
class observable : protected base_observable<Float_Type, Counter_Type>
{
   public:
    typedef base_observable< Float_Type, Counter_Type > base_class;

    typedef typename base_class::mean_type mean_type;

    typedef typename base_class::counter_type counter_type;


   private:
    counter_type count_;

    mean_type max_;

    mean_type min_;


   public:
    observable()
      : base_class ()
      , count_ (0)
      , max_ ()
      , min_ ()
    {}

    observable(const observable<Float_Type, Counter_Type> & other)
      : base_class (other)
      , count_ (other.count_)
      , max_  (other.max_)
      , min_  (other.min_)
    {}

    observable(mean_type first)
      : base_class (first)
      , count_ (1)
      , max_ (first)
      , min_ (first)
    {}

    ~observable()
    {}

    void swap(observable<Float_Type, Counter_Type> & other)
    {
      this->base_class::swap (other);
      std::swap (count_, other.count_);
      std::swap (max_, other.max_);
      std::swap (min_, other.min_);
    }

    observable<Float_Type, Counter_Type> & operator =(observable<Float_Type, Counter_Type> other)
    {
      this->swap(other);
      return *this;
    }

    observable<Float_Type, Counter_Type> & operator<<(mean_type next)
    {
      // Increment counter here as we need the total count when
      // calculating the average.
      ++count_;
      base_class::insert (count_, next);
      if (1 != count_)
      {
        min_ = std::min(next, min_);
        max_ = std::max(next, max_);
      }
      else
      {
        // first value
        min_ = next;
        max_ = next;
      }
      return *this;
    }

    counter_type size() const
    {
      return count_;
    }
    mean_type mean() const
    {
      return base_class::mean ();
    }

    mean_type minimum() const
    {
      return min_;
    }

    mean_type maximum() const
    {
      return max_;
    }

    mean_type variance() const
    {
      return base_class::variance(count_);
    }

    mean_type stddev() const
    {
      return base_class::stddev (count_ );
    }

    void reset()
    {
      *this = observable();
    }


};
// Class: observable_histogram
//
// Manage a value that is derived from the repeated counting of
// observations.
//
// This class uses a successive addition algorithm to calculate the
// mean and variance of a sample value. This is reported to be
// numerically more stable than the traditional sum and sum of
// squares methods.
//
// The default Counter_Type is size_t.  Generally this should be an
// unsigned integer of approximately the same size as the Float_Type
template<class Float_Type, class Counter_Type = std::size_t>
class observable_histogram
{
   public:
    typedef Float_Type mean_type;

    typedef Counter_Type counter_type;

    typedef base_observable< Float_Type, Counter_Type > bin_type;


   private:
    // Number of samples
    counter_type count_;

    std::vector<bin_type> histogram_;

   public:
    observable_histogram()
      : count_ (0)
      , histogram_ ()
    {}

    observable_histogram(std::size_t bincount)
      : count_ (0)
      , histogram_ (bincount)
    {}

    observable_histogram(const observable_histogram<Float_Type, Counter_Type> & other)
      : count_ (other.count_)
      , histogram_ (other.histogram_)
    {}

    ~observable_histogram()
    {}

    void swap(observable_histogram<Float_Type, Counter_Type> & other)
    {
      std::swap (this->count_, other.count_);
      std::swap (this->histogram_, histogram_);
    }

    observable_histogram<Float_Type, Counter_Type> & operator =(observable_histogram<Float_Type, Counter_Type> other)
    {
      this->swap(other);
      return *this;
    }

    // set each histogram bin during a sampling run by
    // value from iterators.
    template<class FwdIter>
    void insert(FwdIter begin, FwdIter end)
    {
      // Increment counter here as we need the total count when calculating the average.
      if (begin != end)
      {
        ++count_;
        for (typename std::vector < observable_histogram< Float_Type, Counter_Type >::bin_type >::iterator iter= histogram_.begin ();
               iter != histogram_.end () and begin != end;
               ++begin, ++iter)
        {
          iter->insert (count_, *begin);
        }
      }
    }

    // The number of samples taken
    counter_type size() const
    {
      return this->count_;
    }

    // The number of histogram bins
    std::size_t bin_count() const
    {
      return this->histogram_.size ();
    }

    // The average value of the sampled distribution
    mean_type mean(std::size_t idx) const
    {
      return this->histogram_[idx].mean ();
    }

    // The sample variance of the sampled distribution
    mean_type variance(std::size_t idx) const
    {
        return histogram_[idx].variance (count_);
    }

    // The standard deviation of the sampled distribution
    mean_type stddev(std::size_t idx) const
    {
        return histogram_[idx].stddev(count_);
    }

    // Reset all the histogram bins
    //
    // @ensure pre(bin_count) == post(bin_count)
    void reset()
    {
        *this = observable_histogram(histogram_.size ());
    }

    //Change ths size of the bins
    //
    // @ensure newsize == bin_count
    void resize(std::size_t newsize)
    {
        this->histogram_.resize (newsize);
    }


};
//Manage a value that is derived from repeated binary (pass/fail) 
//observations.
//
//This class uses a successive addition algorithm to calculate the
//mean and variance of a sample value. This is reported to be
//numerically more stable than the traditional sum and sum of
//squares methods.
//
//The default Counter_Type is size_t.  Generally this should be an
//unsigned integer of approximately the same size as the Float_Type
template<class Float_Type, class Counter_Type = std::size_t>
class binomial_observable
 {
   public:
    typedef Float_Type mean_type;

    typedef Counter_Type counter_type;


   private:
    counter_type count_;

    mean_type mean_;

    counter_type pass_;

    mean_type vars_;


   public:
  inline binomial_observable()
    : count_ (0)
    , pass_ ()
    , vars_ ()
    {}

  inline binomial_observable(const binomial_observable<Float_Type, Counter_Type> & other)
    : count_ (other.count_)
    , pass_ (other.pass_)
    , vars_ (other.vars_)
    {}

    inline ~binomial_observable() {}

    inline void swap(binomial_observable<Float_Type, Counter_Type> & other)
    {
      std::swap (count_, other.count_);
      std::swap (pass_, other.pass_);
      std::swap (vars_, other.vars_);
    }

  inline binomial_observable<Float_Type, Counter_Type> & operator =(binomial_observable<Float_Type, Counter_Type> other)
  {
    this->swap(other);
    return *this;
  }

  inline void fail() 
  {
    // Increment counter here as we need the total count when calculating the average.
    ++count_;
    if (1 != count_)
    {
      vars_ += mean_type(pass_*pass_)/mean_type(count_*count_ - count_);
    }
    // else first value == zero
  }

  inline void pass() 
  {
    // Increment counter here as we need the total count when
    // calculating the average.
    ++count_;
    ++pass_;
    if (1 != count_)
    {
      vars_ += mean_type((count_-pass_)*(count_-pass_))/mean_type(count_*count_ - count_);
    }
    else
    {
      vars_ = mean_type(pass_);
    }
  }

    binomial_observable<Float_Type, Counter_Type> & operator<<(bool passed)
    {
      passed ? pass() : fail ();
      return *this;
    }

  inline mean_type mean() const
  {
    return (pass_ == 0 ? mean_type(0) : mean_type(pass_)/mean_type(count_));
  }

  inline mean_type variance() const
  {
    return (count_ > 1 ? vars_/(mean_type(count_ - 1)) : mean_type(0)); 
  }

    inline counter_type size() const
    {
      return count_;
    }

    inline mean_type stddev() const
    {
      return (count_ > 1 ? std::sqrt(variance()) : mean_type(0));
    }

  inline void reset()
  {
    *this = binomial_observable();
  }

    //Number of passed attempts
    counter_type pass_count() const
    {
      return pass_;
    }

    //Number of failed attempts
    counter_type fail_count() const
    {
      return count_ - pass_;
    }


};

} // namespace ionch::implementation

} // namespace ionch
#endif
