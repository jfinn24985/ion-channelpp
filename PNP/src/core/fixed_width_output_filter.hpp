#ifndef IONCH_CORE_FIXED_WIDTH_OUTPUT_FILTER_HPP
#define IONCH_CORE_FIXED_WIDTH_OUTPUT_FILTER_HPP

//----------------------------------------------------------------------
//This source file is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------

#include <boost/iostreams/concepts.hpp>    // output_filter
#include <cstddef>
#include <string>

// manuals
#include <boost/iostreams/operations.hpp>
//
namespace core {

//Boost IO OutputFilter that allows paragraph formatting, supporting fixed width lines
//and indenting.

class fixed_width_output_filter : public boost::iostreams::output_filter
 {
   public:
    // Create output filter for the given indent size, indent depth
    // and max text width.
    explicit fixed_width_output_filter(std::size_t indent_size, std::size_t depth, std::size_t max_width)
    : current_word_(), col_no_(), depth_( depth ), indent_size_( indent_size )
    , off_( std::string::npos ), spaces_( depth * indent_size ), width_( max_width ), state_(), word_state_()
    {}


   private:
    // The current partial word.
    std::string current_word_;

    // The current text column
    std::size_t col_no_;

    // The current indentation level.
    std::size_t depth_;

    // The number of spaces to indent.  
    std::size_t indent_size_;

    // How much of current_word_ we have processed.  Guard value is string::npos.
    std::string::size_type off_;

    // The current number of spaces to indent. (== indent_size_ * depth_)
    std::size_t spaces_;

    // The target line width.
    std::size_t width_;

    // Current state of the 'put' function.
    std::size_t state_;

    // Current state of the 'write_word' function.
    std::size_t word_state_;


   public:
    // Attempt to write the current word to the sink and reset state.
    template< typename Sink > void close(Sink & dest)
    {
      if( this->off_ == std::string::npos ) { this->off_ = 0; }
      if( not this->current_word_.empty() )
      {
        try
        {
           this->write_word( dest );
        }
        catch(...)
        {
           this->reset();
           throw;
        }
      }
      this->reset();
    }
    // Process the next character from the out going stream.
    template< typename Sink > bool put(Sink & dest, int c)
    {
      while( true ) {
      // std::cout << "DO[" << this->current_word_ << "](" << char(c) << ")#" << this->col_no_ << "S" << this->spaces_ << " F" << this->off_ << "{" << this->state_ <<"}\n"; 
      switch( this->state_ )
      {
         case 0: // beginning of line
           if( '\n' == c )
              return this->put_char( dest, c );
           this->state_ = 1; // fall through
         case 1: // need indent
           if( std::isspace( c ) ) return true; // ignore spaces at indent
           if( '\n' == c )
           {
              this->state_ = 0;
              return this->put_char( dest, c );
           }
           while( this->col_no_ < this->spaces_ )
           {
             if( not this->put_char(dest, ' ') ) return false;
           }
           this->state_ = 2; // fall through
         case 2: // begin entry
           if( std::isalpha( c ) )
             this->state_ = 3; // (in word)
           else if( std::isdigit( c ) or '-' == c )
             this->state_ = 4; // (start number)
           else if( '.' == c )
             this->state_ = 5; // (in number fraction (OR PUNCT))
           else if( '\n' == c )
             {
               this->state_ = 0; // (in number fraction (OR PUNCT))
               return this->put_char( dest, c );
             }
           else
             {
               if( this->col_no_ == this->width_ ) {
                 this->state_ = 8;
                 break;
               }
               return this->put_char( dest, c );
             }
           this->current_word_.append( 1ul, static_cast< unsigned char >( c ) );
           return true;
         case 3: // in word
           if( not std::isalpha( c ) )
           {
              this->state_ = 9; // (end entry)
              break;
           }
           this->current_word_.append( 1ul, static_cast< unsigned char >( c ) );
           return true;
         case 4: // start number
           if( not std::isdigit( c ) ) {
             if( '.' == c ) this->state_ = 5; // (in number fraction)
             else if( 'd' == c or 'D' == c or 'e' == c or 'E' == c ) this->state_ = 6; // (in number start base)
             else {
               this->state_ = 9; // (end entry)
               break;
             }
           }
           this->current_word_.append( 1ul, static_cast< unsigned char >( c ) );
           return true;
         case 5: // number fraction
           if( not std::isdigit( c ) ) {
             if( 'd' == c or 'D' == c or 'e' == c or 'E' == c ) this->state_ = 6; // (in number start base)
             else {
               this->state_ = 9; // (end entry)
               break;
             }
           }
           this->current_word_.append( 1ul, static_cast< unsigned char >( c ) );
           return true;
         case 6: // number start base
           this->state_ = 7; // (number base)
           if( not std::isdigit( c ) or '-' != c ) {
               this->state_ = 9; // (end entry)
               break;
             }
           this->current_word_.append( 1ul, static_cast< unsigned char >( c ) );
           return true;
         case 7: // number start base
           if( not std::isdigit( c ) ) {
               this->state_ = 9; // (end entry)
               break;
             }
           this->current_word_.append( 1ul, static_cast< unsigned char >( c ) );
           return true;
         case 8: // wait for new line
           if( not this->put_char( dest, '\n' ) ) return false;
           this->state_ = 0; // (beginning of line)
           break;
         case 9: // end entry 
           if( not this->write_word( dest ) ) return false;
           // write word sets state.
           break;
         default: // ERROR
           throw std::runtime_error( "Should never get to default case" );
        }
      } while( true );
      // never get here!
      return true;
    }

   private:
    // Attempt to write the current word to the sink.
    template< typename Sink > bool write_word(Sink & dest)
    {
      if( this->off_ == std::string::npos ) this->off_ = 0;
      const std::size_t left( this->current_word_.size() - this->off_ );
      if( this->col_no_ != this->spaces_ and this->col_no_ + left > this->width_ ) this->word_state_ = 1;
      while( this->off_ != std::string::npos )
      {
        switch( this->word_state_ )
        {
        case 1: // need new line
          if( not this->put_char( dest, '\n' ) ) return false;
          this->word_state_ = 2; // (fall through)
        case 2:
          while( this->col_no_ < this->spaces_ )
          {
            if( not this->put_char(dest, ' ') ) return false;
          }
          this->word_state_ = 0; // (fall through)
        case 0: // write word
            while(this->off_ < this->current_word_.size()) {
              // handle words too long for one line or end on the line boundary
              if( this->col_no_ == this->width_ ) {
                this->word_state_ = 1;
                break;
              }
              if( not this->put_char( dest, this->current_word_[ this->off_ ] ) )
              { return false; }
              ++this->off_;
            }
            if( this->current_word_.size() == this->off_ ) this->off_ = std::string::npos;
          break;
        default: // ERROR
           throw std::runtime_error( "Should never get to default case" );
        }
      }
      if( this->off_ == std::string::npos )
      {
        this->current_word_.clear();
        this->state_ = 2; // begin entry
      }
      return true;
    }
    // Writes the given character to Sink and updates column number.
    template< typename Sink > bool put_char(Sink & dest, int c)
    {
      // std::cout << "PUTCHAR[" << this->current_word_ << "](" << char(c) << ")#" << this->col_no_ << "S" << this->spaces_ << " F" << this->off_ << "\n"; 
      if( not boost::iostreams::put( dest, c ) ) return false;
      if( c == '\n' ) { this->col_no_ = 0; }
      else            { ++this->col_no_; }
      return true;
    }


   public:
    // Get the current indentation level
    std::size_t depth() const
    {
      return this->depth_;
    }

    // Increase indentation level by one.
    void increment_depth()
    {
      ++this->depth_;
      this->spaces_ += this->indent_size_;
    }

    // Increase indentation level by one.
    void decrement_depth()
    {
      if( this->depth_ != 0ul )
      {
        --this->depth_;
        this->spaces_ -= this->indent_size_;
      }
    }

    // Get the current indentation level
    std::size_t text_width() const
    {
      return this->width_;
    }

    // Get the current indentation level
    std::size_t indent_size() const
    {
      return this->indent_size_;
    }

    // Return attributes ready for restarting formatting.
    void reset()
    {
     if( not this->current_word_.empty() ) this->current_word_.clear();
     this->off_ = std::string::npos;
     this->col_no_ = 0ul;
     this->state_ = 0ul;
     this->word_state_ = 0ul;
    }


};
// Class that automatically increments and decrements the indent of a filter.
class indent_guard
 {
   private:
    // The filter we increment/decrement the indent for
    fixed_width_output_filter *filter_;


   public:
    indent_guard(fixed_width_output_filter & filt)
    : filter_( &filt )
    {
      filt.increment_depth();
    }

    ~indent_guard()
    {
      this->filter_->decrement_depth();
    }


   private:
    indent_guard(indent_guard && source);

    indent_guard(const indent_guard & source);

    indent_guard & operator=(const indent_guard & source);


};

} // namespace core
#endif
