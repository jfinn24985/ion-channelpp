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


#ifndef DEBUG
#define DEBUG 0
#endif

#include "CC/implementation/observable.hpp"

namespace ionch {

namespace implementation {


} // namespace ionch::implementation

} // namespace ionch
