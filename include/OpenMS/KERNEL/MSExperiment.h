// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// --------------------------------------------------------------------------
//                   OpenMS Mass Spectrometry Framework
// --------------------------------------------------------------------------
//  Copyright (C) 2003-2006 -- Oliver Kohlbacher, Knut Reinert
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
// 
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// --------------------------------------------------------------------------
// $Id: MSExperiment.h,v 1.7 2006/06/09 23:47:35 nicopfeifer Exp $
// $Author: nicopfeifer $
// $Maintainer: Marc Sturm $
// --------------------------------------------------------------------------

#ifndef OPENMS_KERNEL_MSEXPERIMENT_H
#define OPENMS_KERNEL_MSEXPERIMENT_H

#include <OpenMS/KERNEL/MSSpectrum.h>
#include <OpenMS/METADATA/ExperimentalSettings.h>
#include <OpenMS/KERNEL/DimensionDescription.h>
#include <OpenMS/FORMAT/PersistentObject.h>
#include <OpenMS/CONCEPT/Exception.h>

#include<vector>
#include<algorithm>
#include<limits>

namespace OpenMS 
{
	/**
		@brief Representation of a mass spectrometry experiment.
		
		Contains the data and metadata of an experiment performed with an MS (or HPLC and MS).
		
		Be carefull when changing the order of contained MSSpectrum instances, if tandem-MS data is
		stored in this class. The only way to find a precursor spectrum of MSSpectrum x is to 
		search for the first spectrum before x that has a lower MS-level!
		
		@note For range operations, see \ref RangeUtils "RangeUtils module"!
		
		@note To iterate over the peaks in all spactra use PeakIterator
		
		@ingroup Kernel
	*/
	template <typename PeakT = DPeak<1> >
  class MSExperiment
 		: public std::vector<MSSpectrum<PeakT> >, 
  		public ExperimentalSettings,
  		public PersistentObject
  {
    public:
		typedef MSSpectrum<PeakT> SpectrumType;
		typedef typename std::vector< MSSpectrum<PeakT> >::iterator Iterator;
    typedef typename std::vector< MSSpectrum<PeakT> >::const_iterator ConstIterator;
    typedef PeakT PeakType;
		typedef typename PeakType::CoordinateType CoordinateType;  	
    typedef std::vector<MSSpectrum<PeakT> > Base;

  	/// Constructor
    MSExperiment()
			: std::vector<MSSpectrum<PeakT> >(),
			ExperimentalSettings(),
			PersistentObject(),
			mz_min_(0),
			mz_max_(0),
			rt_min_(0),
			rt_max_(0),
			it_min_(0),
			it_max_(0),
			ms_levels_(),
			nr_dpoints_(0),
			spectra_lengths_(),
			name_()
		{			 
		}
		
    /// Copy constructor
    MSExperiment(const MSExperiment& source):
			std::vector<MSSpectrum<PeakT> >(source),
			ExperimentalSettings(source),
			PersistentObject(source),
			mz_min_(source.mz_min_),
			mz_max_(source.mz_max_),
			rt_min_(source.rt_min_),
			rt_max_(source.rt_max_),
			it_min_(source.it_min_),
			it_max_(source.it_max_),
			ms_levels_(source.ms_levels_),
			nr_dpoints_(source.nr_dpoints_),
			spectra_lengths_(source.spectra_lengths_),
			name_(source.name_)
		{			  
		}

    /// Destructor
    ~MSExperiment()
		{
		  
		}
    
    /// Assignment operator
    MSExperiment& operator= (const MSExperiment& source)
		{
		  if (&source == this) return *this;
		  
		  std::vector<MSSpectrum<PeakT> >::operator=(source);
		  ExperimentalSettings::operator=(source);
		  PersistentObject::operator=(source);
		  name_ = source.name_;
		  return *this;
		}
    
    /// Equality operator
    bool operator== (const MSExperiment& rhs) const
	  {
	  	return ExperimentalSettings::operator==(rhs) && std::operator==(rhs,*this);
	  }
    /// Equality operator
    bool operator!= (const MSExperiment& rhs) const
	  {
	  	return !(operator==(rhs));
	 	}     
     
		/**
			@brief Reads out a 2D Spectrum
    	
    	Container is a DPeakArray<2>, DPeakList<2>, DPeakArrayNonPolymorphic
			or a STL container of DPeak<2> or DRawDataPoint<2> which supports insert(), end() and back()
    */
    template <class Container>
    void get2DData(Container& cont) const
	  {
			const int MZ = DimensionDescription < DimensionDescriptionTagLCMS >::MZ;
			const int RT = DimensionDescription < DimensionDescriptionTagLCMS >::RT;
		  	
			for (typename Base_::const_iterator spec = Base_::begin(); spec != Base_::end(); ++spec)
			{
				if (spec->getMSLevel()==1)
				{
					for (typename MSSpectrum<PeakT>::const_iterator it = spec->begin(); it!=spec->end(); ++it)
					{
						cont.insert(cont.end(), typename Container::value_type());
						cont.back().getPosition()[RT] = spec->getRetentionTime();
						cont.back().setIntensity(it->getIntensity());
						cont.back().getPosition()[MZ] = it->getPosition()[0];
					}
				}
			}
	  }

		/**
			@brief Assignment of a 2D spectrum to MSExperiment
			  	
			Container is a DPeakArray<2>, DPeakList<2>, DPeakArrayNonPolymorphic
			or a STL container of DPeak<2> or DRawDataPoint<2>
			
			@note The container has to be sorted according to retention time. Otherwise a Precondition exception is thrown.
		*/				
	  template <class Container> void set2DData(const Container& cont) throw (Exception::Precondition)
		{
			SpectrumType* spectrum;
		 	/// If the container is emptry, nothing will happen
			if (cont.size() == 0)  return;
		  
			const int MZ = DimensionDescription < DimensionDescriptionTagLCMS >::MZ;
			const int RT = DimensionDescription < DimensionDescriptionTagLCMS >::RT;
		  
		  typename PeakType::CoordinateType current_rt = -1.0*std::numeric_limits<typename PeakType::CoordinateType>::max();
		 	
	  	for (typename Container::const_iterator iter = cont.begin(); iter != cont.end(); iter++)
	  	{
	  		// check if the retentime time has changed
	  		if (current_rt != iter->getPosition()[RT])
	  		{
	  			if (current_rt > iter->getPosition()[RT])
	  			{
	  				throw Exception::Precondition(__FILE__, __LINE__, __PRETTY_FUNCTION__,"Input container is not sorted!");
	  			}
	  			current_rt =  iter->getPosition()[RT];
	  			Base_::insert(Base_::end(),SpectrumType());
	  			spectrum = &(Base_::back());
	  			spectrum->setRetentionTime(current_rt);
	  			spectrum->setMSLevel(1);
	  		}
	  		
	  		// create temporary peak and insert it into spectrum
	  		spectrum->insert(spectrum->end(), PeakType());
	  		spectrum->back().setIntensity(iter->getIntensity());
				spectrum->back().getPosition()[0] = iter->getPosition()[MZ]; 
	  	}
		}

		/// Returns the name
		const String& getName() const
		{
			return name_;
		}

		/// Sets the name
		void setName(const String& name)
		{
			name_ = name;
		}
			
		/**
			@brief Adaptor class for bidirectional iterator on objects of DPeak<1>  
			
			This iterator allows us to move through the data structure in a linear
			manner i.e. we don't need to jump to the next spectrum manually.
			
			The class has a member  DPeakArrayNonPolymorphic<>::iterator pointing
			to the current peak. The class also remembers the retention time of the current 
			scan.			
		*/	
		class PeakIterator : public std::iterator<std::bidirectional_iterator_tag,  PeakType>
		{
			typedef double CoordinateType;
			typedef DPeakArrayNonPolymorphic<1, DPeak<1> >::iterator IteratorType;
			
			public:
					
				/// Default constructor
				PeakIterator() 
					: iter_(), rt_(), index_(), exp_() 
				{
				
				}
				
				/// Constructor
				PeakIterator(IteratorType it, CoordinateType co, UnsignedInt ind, MSExperiment<PeakType>& exp) 
					: iter_(it), rt_(co), index_(ind), exp_(&exp) 
				{
					
				}
				
				/// Destructor 
				~PeakIterator() 
				{
					
				}
				
				/// Copy constructor
				PeakIterator(const PeakIterator& rhs)
					: iter_(rhs.iter_), rt_(rhs.rt_), 
					  index_(rhs.index_), exp_(rhs.exp_) 
				{ }
						
				/// Assignment operator
				PeakIterator& operator=(const PeakIterator& rhs)
				{
					if (&rhs == this) return *this;			
						
					iter_    = rhs.iter_;
					rt_       = rhs.rt_;
					index_ = rhs.index_;
					exp_    = rhs.exp_;
				
					return (*this);
				}
				
				/// Test for equality
				friend bool operator==(const PeakIterator& lhs, const PeakIterator& rhs)
				{
					return ( lhs.iter_     == rhs.iter_ &&
					     lhs.rt_       == rhs.rt_       && 
					     lhs.index_ == rhs.index_ );
				}
				
				/// Test for inequality
				bool operator!=(const PeakIterator& rhs)
				{
					return !(*this  == rhs);
				}
				
				/// Step forward by one (prefix operator)
				PeakIterator& operator++()
				{
					++iter_;
					// test whether we arrived at the end of the current scan
					if ( iter_ ==   (*exp_)[index_].end() && index_ < (exp_->size()-1 ) )
					{	
						// set internal iterator to start of new scan and update retention time
						iter_ = (*exp_)[++index_].begin() ; 
						rt_   = (*exp_)[index_].getRetentionTime();
					}
					return (*this); 
				}	
				
				/// Step backward by one (prefix operator)
				/// What happens if we are the beginning of a scan and move backwards ???
				PeakIterator& operator--()
				{
					--iter_;
					// test whether we are at the start of a scan
					if ( *iter_ ==   *(exp_->at(index_).begin())  && index_ > 0 )
					{	
						// set internal iterator to end of new scan and update retention time
						iter_ =  ((*exp_)[--index_].end() -1) ; 
						rt_   = (*exp_)[index_].getRetentionTime();
						return (*this); 
					}
					return (*this); 
				}	
				
				/// Step forward by one (postfix operator)
				PeakIterator operator++(int)
				{
					PeakIterator tmp(*this);
					++(*this);
					return tmp;
				}
				
				
				/// Step backward by one (postfix operator)
				PeakIterator operator--(int)
				{
					PeakIterator tmp(*this);
					--(*this);
					return tmp;				
				}
				
				/// Dereferencing of this pointer yields the underlying peak
				PeakType& operator * () 
				{
					return (*iter_);
				}
				
				/// Dereferencing of this pointer yields the underlying peak
				PeakType* operator-> () 
				{
					return &(*iter_);
				}
				
				UnsignedInt getIndexInExp()
				{
					UnsignedInt i=0;
					UnsignedInt dist = 0;
					while (i != index_)
					{
						dist += (*exp_)[i++].size();
					}
					dist = (iter_ - (*exp_)[i].begin());					
					
					return dist;
				}
				
				/** @name Accesssors
				*/
				//@{	
				/// Returns the current retention time (mutable)
				CoordinateType& getRt() { return rt_; }
				/// Returns the current retention time (not mutable)
				const CoordinateType& getRt() const { return rt_; }
				///Returns the current retention time (mutable)
				UnsignedInt& getIndex() { return index_; }
				/// Returns the current retention time (not mutable)
				const UnsignedInt& getIndex() const { return index_; }
				//@} 
						
				private:
				/// Points to the current peak
				IteratorType iter_;
				/// Retention time of the current spectrum
				CoordinateType rt_;
				/// Index of the current spectrum 
				UnsignedInt index_;   
				/// Pointer to the experiment
				MSExperiment<PeakType> * exp_;
			
			}; // end of inner class PeakIterator
				 
				
			/// Returns an iterator pointing at the first peak			 
			PeakIterator RTBegin()
			{
				return (PeakIterator(this->at(0).begin() , this->at(0).getRetentionTime(), 0 , *this) );
			}
	
			/// Returns an iterator pointing at the last peak
			PeakIterator RTEnd()
			{
				return(PeakIterator( (this->at(this->size()-1).end()), this->at(this->size()-1).getRetentionTime(), (this->size()-1), *this ) );
			}			
			
			/**	
				@name Range methods  
				
				@note The range values (min, max, etc.) are not updated automatically. Call updateRanges() to update the values!
			*/
			//@{

			/// updates the m/z, intensity, retention time and MS level ranges
			void updateRanges()
			{
				//clear MS levels
				ms_levels_.clear();
				// clear spectra lengths
				spectra_lengths_.clear();
				spectra_lengths_.reserve(this->size());
				
				//empty
				if (this->size()==0)
				{
					mz_min_ = mz_max_ = 0;
					rt_min_ = rt_max_ = 0;
					it_min_ = it_max_ = 0;
					return;
				}
				
				//temporary variable
				double rt;
			
				//initialize
				rt_min_ = rt_max_ = this->begin()->getRetentionTime();				
				mz_min_ = std::numeric_limits<typename SpectrumType::PeakType::CoordinateType>::max();
				mz_max_ = -1.0 * std::numeric_limits<typename SpectrumType::PeakType::IntensityType>::max();
				it_min_ = std::numeric_limits<typename SpectrumType::PeakType::CoordinateType>::max();
				it_max_ = -1.0 * std::numeric_limits<typename SpectrumType::PeakType::IntensityType>::max();
				nr_dpoints_ = 0;
				
				//update
				for (typename std::vector<MSSpectrum<PeakT> >::iterator it = this->begin(); it!=this->end(); ++it)
				{
					//rt
					rt = it->getRetentionTime();
					if (rt < rt_min_) rt_min_ = rt;
					if (rt > rt_max_) rt_max_ = rt;
					
					//ms levels
					if (std::find(ms_levels_.begin(),ms_levels_.end(),it->getMSLevel())==ms_levels_.end())
					{
						ms_levels_.push_back(it->getMSLevel());
					}
					
					it->updateRanges();
					
					//mz
					if (it->getMin()[0] < mz_min_) mz_min_ = it->getMin()[0];
					if (it->getMax()[0] > mz_max_) mz_max_ = it->getMax()[0];
					
					//int
					if (it->getMinInt() < it_min_) it_min_ = it->getMinInt();			
					if (it->getMaxInt() > it_max_) it_max_ = it->getMaxInt();
					
					// calculate size
					nr_dpoints_ += it->size();
					spectra_lengths_.push_back(it->size());
										
				}
				std::sort(ms_levels_.begin(), ms_levels_.end());
				
				std::vector<UnsignedInt>::const_iterator citer = spectra_lengths_.begin();
				
			}
			
			/// returns the minimal m/z value
			typename SpectrumType::PeakType::CoordinateType getMinMZ() const
			{
				return mz_min_;
			}

			/// returns the maximal m/z value
			typename SpectrumType::PeakType::CoordinateType getMaxMZ() const
			{
				return mz_max_;
			}

			/// returns the minimal retention time value
			double getMinRT() const
			{
				return rt_min_;
			}
			
			/// returns the maximal retention time value
			double getMaxRT() const
			{
				return rt_max_;
			}

			/// returns the minimal intensity value
			double getMinInt() const
			{
				return it_min_;
			}
			
			/// returns the maximal intensity value
			double getMaxInt() const
			{
				return it_max_;
			}
			
			/// returns the number of peaks in all spectra
			UnsignedInt getSize() const
			{
				return nr_dpoints_;
			}

			/// returns an array of MS levels
			const std::vector<UnsignedInt>& getMSLevels() const
			{
				return ms_levels_;
			}
						
			/// Access to peak with index @p index
			DPeak<2> getPeak(const UnsignedInt index) throw (Exception::IndexOverflow) 
			{ 
				if (index > nr_dpoints_) throw Exception::IndexOverflow(__FILE__, __LINE__, __PRETTY_FUNCTION__, index, nr_dpoints_);
				
				const int MZ = DimensionDescription < DimensionDescriptionTagLCMS >::MZ;
				const int RT = DimensionDescription < DimensionDescriptionTagLCMS >::RT;
				
				UnsignedInt spectra_nr = 0; 
				UnsignedInt peak_nr    = index;
				while ( spectra_nr < spectra_lengths_.size() && peak_nr >= spectra_lengths_[spectra_nr] )
				{
					peak_nr -= spectra_lengths_[spectra_nr++];	
				}	
						
				DPeak<2> tmp;
				tmp.getPosition()[RT] =  (*this)[spectra_nr].getRetentionTime();
				tmp.getPosition()[MZ] = (*this)[spectra_nr].getContainer()[peak_nr].getPosition()[0];
				tmp.setIntensity((*this)[spectra_nr].getContainer()[peak_nr].getIntensity());
				
				return tmp;
			}
			
			/// Access to peak with index @p index
			const DPeak<2> getPeak(const UnsignedInt index) const throw (Exception::IndexOverflow) 
			{ 
				if (index > nr_dpoints_) throw Exception::IndexOverflow(__FILE__, __LINE__, __PRETTY_FUNCTION__, index, nr_dpoints_);
				
				const int MZ = DimensionDescription < DimensionDescriptionTagLCMS >::MZ;
				const int RT = DimensionDescription < DimensionDescriptionTagLCMS >::RT;
				
				UnsignedInt spectra_nr = 0; 
				UnsignedInt peak_nr    = index;
				while ( spectra_nr < spectra_lengths_.size() && peak_nr >= spectra_lengths_[spectra_nr] )
				{
					peak_nr -= spectra_lengths_[spectra_nr++];	
				}	
						
				DPeak<2> tmp;
				tmp.getPosition()[RT] =  (*this)[spectra_nr].getRetentionTime();
				tmp.getPosition()[MZ] = (*this)[spectra_nr].getContainer()[peak_nr].getPosition()[0];
				tmp.setIntensity((*this)[spectra_nr].getContainer()[peak_nr].getIntensity());
				
				return tmp;
			}
			//@}

			/// PersistentObject interface
			virtual void persistentWrite(PersistenceManager& pm, const char* name=0) const throw (Exception::Base)
			{
				//std::cout << "--  MSExperiment Header --" << std::endl;
				pm.writeObjectHeader(this,name);
				//std::cout << "--  MSExperiment -> Spectrum Array --" << std::endl;
				pm.writeObjectArray(*(dynamic_cast< const std::vector<MSSpectrum<PeakT> >* >(this)),"SpectrumArray",this->size());
				//std::cout << "--  MSExperiment Tailer --" << std::endl<< std::endl<< std::endl;
				pm.writeObjectTrailer(name);
			}
			
			/// PersistentObject interface
			virtual void persistentRead(PersistenceManager& pm) throw (Exception::Base)
			{
				pm.readPrimitive(getPersistenceId(),"id");
				
				//spectra
				UnsignedInt tmp;
				pm.readPrimitive(tmp,"spectrum count");
				this->resize(tmp);
				for (UnsignedInt i=0; i<tmp; ++i)
				{
					pm.readObjectReference(this->operator[](i),"Spectrum");
				}
			}
			
			/// Comparator to sort spectra by retention time
			class RtComparator
			{
				public : 
					template<typename SpectrumT>			
 					bool 	operator () (SpectrumT& left, SpectrumT& right) const
					{
						return left.getRetentionTime() < right.getRetentionTime();
					}
			};
			
			/** 
				@brief Sorts the data points by retention time
								
				@p sort_mz : indicates whether the points should be sorted by m/z as well			
			*/
			void sortSpectra(bool sort_mz)
			{
			
				RtComparator comp;
				std::sort(this->begin(),this->end(),comp);
			
				if (sort_mz)
				{
					// sort each spectrum by m/z
					for (Iterator iter = this->begin();
						  iter != this->end();
						  ++iter)
					{
						iter->getContainer().sortByPosition();
					}
				}
			}
			
   	/// returns a const reference to the protein identification vector
   	const std::vector<ProteinIdentification>& getProteinIdentifications() const
   	{
	  	return protein_identifications_;	   		
   	}	
   		    	
   	/// returns a mutable reference to the protein identification vector
	  std::vector<ProteinIdentification>& getProteinIdentifications()
	  {
	  	return protein_identifications_;	
	  }
	  
	  /// sets the protein identification vector
	  void setProteinIdentifications(const std::vector<ProteinIdentification>& protein_identifications)
	  {
	  	protein_identifications_ = protein_identifications;
	  }
	  
	  /// adds an identification to the identification vector
	  void addProteinIdentifications(ProteinIdentification& protein_identification)
	  {
	  	protein_identifications_.push_back(protein_identification);
	  }


		protected:
		
			///PersistentObject interface
	    virtual void clearChildIds_()
	    {
	    	//TODO Persistence	
	    };	

			/// boundaries of m/z 
			typename SpectrumType::PeakType::CoordinateType mz_min_, mz_max_;
			///
			typedef typename std::vector<MSSpectrum<PeakT> > Base_;
			/// boundaries of retention time
			double rt_min_, rt_max_;
			/// boundaries of the intensity
			double it_min_, it_max_;
			/// MS levels of the data
			std::vector<UnsignedInt> ms_levels_;
			/// Number of all data points
			unsigned int nr_dpoints_;
			/// Length of each spectrum
			std::vector<UnsignedInt> spectra_lengths_;
			/// Retention time of last retrieved peak
			CoordinateType last_rt_;
			/// Protein identifications
			std::vector<ProteinIdentification> protein_identifications_;
			/// Name string
			String name_;
					
	};

	///Print the contents to a stream.
	template <typename PeakT>
	std::ostream& operator << (std::ostream& os, const MSExperiment<PeakT>& exp)
	{
		os << "-- MSEXPERIMENT BEGIN --"<<std::endl;

		//experimental settings
		os <<static_cast<const ExperimentalSettings&>(exp);
		
		//spectra
		for (typename MSExperiment<PeakT>::const_iterator it=exp.begin(); it!=exp.end(); ++it)
		{
			os << *it;
		}

		os << "-- MSEXPERIMENT END --"<<std::endl;
	
		return os;
	}

} // namespace OpenMS

#endif // OPENMS_KERNEL_MSEXPERIMENT_H
