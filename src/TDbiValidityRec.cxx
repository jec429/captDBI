// $Id: TDbiValidityRec.cxx,v 1.2 2012/06/14 10:55:23 finch Exp $

#include "DbiDetector.hxx"
#include "DbiSimFlag.hxx"
#include "TDbiBinaryFile.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiOutRowStream.hxx"
#include "TDbiValidityRec.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
using std::ostringstream;
using std::setw;
using std::setfill;
using std::setprecision;

#include "TVldRange.hxx"
#include "TVldTimeStamp.hxx"

#include "TString.h"

#include <sstream>
#include <iostream>

ClassImp(ND::TDbiValidityRec)


//   Definition of static data members
//   *********************************


//  Instantiate associated Result Pointer class.
//  *******************************************

#include "TDbiResultSetHandle.tpl"
template class  ND::TDbiResultSetHandle<ND::TDbiValidityRec>;

//  Global functions
//  *****************

//.....................................................................

ostream& operator<<(ostream& os, const ND::TDbiValidityRec& vRec) {
//
//
//  Purpose:  Output validity record to ostream.
//
//  Arguments:
//    os           in    output stream
//    vRec         in    Validity record to be output
//
//  Return:        updated ostream
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Output validity record to ostream.

//  Program Notes:-
//  =============

//  None.

  const ND::TVldRange&           range(vRec.GetVldRange());

  ostringstream os2;  //Use local to avoid manipulators from making permanent change to os.
  os2 << "SeqNo: " << vRec.GetSeqNo()
      << "  AggNo: " << vRec.GetAggregateNo()
      << "  DbNo: " << vRec.GetDbNo();
  if ( vRec.IsGap() ) os2 << " (gap)";
  os2  << "  ND::TVldRange: |"
       << setprecision(3) << std::ios::hex << setfill ('0') << "0x" <<  setw(3) << range.GetDetectorMask() << "|"
       << "0x" << setw(3) << range.GetSimMask() << "|"
       << " " << range.GetTimeStart().AsString("s") << " .. "
       << range.GetTimeEnd().AsString("s")
       << " Epoch: " << vRec.GetEpoch()
       << " Task: " << vRec.GetTask()
       << " from: " << range.GetDataSource();
  os << os2.str();
  return os;

}

// Definition of member functions (alphabetical order)
// ***************************************************


//.....................................................................

void ND::TDbiValidityRec::AndTimeWindow(const ND::TVldTimeStamp& startOther,
                                   const ND::TVldTimeStamp& endOther) {
//
//
//  Purpose:  And in supplied start and end times.
//
//  Arguments:
//    startOther   in    Start time to be ANDed in.
//    endOther     in    End time to be ANDed in.
//
//  Return:   None.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Trim time windown on current record to be the overlap (and)
//    with the supplied time window.

//  Program Notes:-
//  =============

//  None.

  ND::TVldTimeStamp start = fVldRange.GetTimeStart();
  ND::TVldTimeStamp end   = fVldRange.GetTimeEnd();

  SetTimeWindow( start > startOther ? start : startOther,
                 end   < endOther   ? end   : endOther );
}

//.....................................................................

ND::TDbiValidityRec::TDbiValidityRec(Int_t dbNo,Bool_t isGap) :

fAggregateNo(-2),
fDbNo(dbNo),
fIsGap(isGap),
fTask(0),
fSeqNo(0),
fTableProxy(0),
fEpoch(0),
fReality(0)
{
//
//
//  Purpose:  Default constructor
//
//  Arguments:.
//    dbNo     in    Cascade no. of database holding record.
//    isGap    in    = kTRUE if a gap.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ValidityRec.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating ND::TDbiValidityRec" << "  ");
}
//.....................................................................

ND::TDbiValidityRec::TDbiValidityRec(const ND::TDbiValidityRec& from) : ND::TDbiTableRow(from)
{
//
//
//  Purpose:  Copy constructor
//
//  Arguments:
//    from    in    Object to be copied from.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ValidityRec.


//  Program Notes:-
//  =============

//  Explicit just for leak checking.


  DbiTrace( "Creating ND::TDbiValidityRec" << "  ");

  *this = from;

}
//.....................................................................

ND::TDbiValidityRec::TDbiValidityRec(const ND::TVldRange& range,
                               TDbi::Task task,
                               Int_t aggNo,
                               UInt_t seqNo,
                               Int_t dbNo,
                               Bool_t isGap,
                               ND::TVldTimeStamp time,
			       UInt_t epoch) :
fAggregateNo(aggNo),
fCreationDate(time),
fDbNo(dbNo),
fIsGap(isGap),
fTask(task),
fSeqNo(seqNo),
fTableProxy(0),
fVldRange(range),
fEpoch(epoch),
fReality(0)
{
//
//
//  Purpose:  Constructor from components
//
//  Arguments:
//    range    in   component range
//    task     in   component task
//    aggNo    in   component aggregate number
//    seqNo    in   component sequence number
//    dbNo     in    Cascade no. of database holding record.
//    isGap    in    = kTRUE if a gap.
//    epoch    in   component calibration epoch
//
//
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Create ValidityRec.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Creating ND::TDbiValidityRec" << "  ");
}

//.....................................................................

ND::TDbiValidityRec::~TDbiValidityRec() {
//
//
//  Purpose: Destructor
//
//  Arguments:
//    None.
//
//  Return:    n/a
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Destroy ValidityRec.


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Destroying ND::TDbiValidityRec" << "  ");

}

//.....................................................................

void ND::TDbiValidityRec::Fill(ND::TDbiInRowStream& rs,
                          const ND::TDbiValidityRec* /* vrec */) {
//
//
//  Purpose:  Fill oject from Result Set
//
//  Arguments:
//    rs           in    Result Set used to fill object
//    vrec         in    =0.  If filling other table rows it points
//                       to the associated validity record.
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Fill object from current row of Result Set.

//  Program Notes:-
//  =============

//  None.

    ND::TVldTimeStamp start, end;
    Int_t detMask, simMask;

//  Establish source ND::TDbiTableProxy and cascade no.

    fTableProxy = rs.GetTableProxy();
    fDbNo       = rs.GetDbNo();

//  Read data from row.

    rs >> fSeqNo >> start >> end;

    if ( rs.HasEpoch() ) rs >> fEpoch >> fReality;
    else fEpoch = fReality = 0;

    rs >> detMask >> simMask >> fTask
       >> fAggregateNo    >> fCreationDate  >> fInsertDate;

    fIsGap = kFALSE;

    ND::TVldRange vr(detMask, simMask, start, end, "From Database");
    fVldRange = vr;

    DbiVerbose(  "ND::TDbiValidityRec for row " << rs.CurRowNum()
	 << ": " << fVldRange.AsString()
	 << " seq num: " << fSeqNo
	 << " agg no: "  << fAggregateNo
	 << " task: " << "  ");
}

//.....................................................................

std::string ND::TDbiValidityRec::GetL2CacheName() const {
//
//
//  Purpose:  Return the associated Level 2 Cache Name.
//
//  Specification:-
//  =============
//
//  o See GetL2CacheName(UInt_t seqLo, UInt_t seqHi, ND::TVldTimeStamp ts);

  return ND::TDbiValidityRec::GetL2CacheName(this->GetSeqNo(),
                                        this->GetSeqNo(),
					this->GetCreationDate());

}
//.....................................................................

std::string ND::TDbiValidityRec::GetL2CacheName(UInt_t seqLo,
                                           UInt_t seqHi,
                                           ND::TVldTimeStamp ts) {
//
//
//  Purpose:  Return the associated Level 2 Cache Name.
//
//  Specification:-
//  =============
//
//  o For unaggregated queries name format: <Seqno>_<CreationDate>
//
//  o For aggregated queries name format:   <LowSeqno>_<HighSeqNo>_<MaxCreationDate>

//  Program Notes:-
//  =============

//  The static routine centralises the definition of cache names both for
//  ND::TDbiValidityRec and ND::TDbiValidityRecBuilder.

  ostringstream oss;
  oss << seqLo << "_";
  if ( seqLo != seqHi )  oss << seqHi << "_";
  oss << ts.AsString("s");
  std::string str(oss.str());
  // Convert white space to underscore.
  int i = str.size();
  while ( i-- ) if ( str[i] == ' ' ) str[i] = '_';
  return str;

}

//.....................................................................

Bool_t ND::TDbiValidityRec::HasExpired(const ND::TDbiValidityRec& other) const {
//
//
//  Purpose: See if validity record has expired i.e. is compatible with
//           this one except for date.
//
//  Arguments:
//    other        in    Other ND::TDbiValidityRec to be compared with
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o See if validity record has expired i.e. is compatible with
//    this one except for date.

//  Program Notes:-
//  =============

//  This method is used by ND::TDbiCache to select candidates for
//  purging.

  const ND::TVldRange& otherVR =other.GetVldRange();

  return
     (       other.GetTask() == fTask
        && ( otherVR.GetDetectorMask() & fVldRange.GetDetectorMask() )
        && ( otherVR.GetSimMask()      & fVldRange.GetSimMask() )
        && (    otherVR.GetTimeStart() >= fVldRange.GetTimeEnd()
             || otherVR.GetTimeEnd()   <= fVldRange.GetTimeStart() )
     );

}
//.....................................................................


Bool_t ND::TDbiValidityRec::HasExpired(const ND::TVldContext& vc,
                                  const TDbi::Task& task) const {
//
//
//  Purpose: See if validity record has expired i.e. is compatible with
//           this context query except for date.
//
//  Arguments:
//    vc           in    Context of query
//    task         in    Task of query
//
//
//  Contact:   N. West

//  Program Notes:-
//  =============

//  This method is used by ND::TDbiResultSet to identify when ready for
//  purging.

  ND::TVldTimeStamp    ts = vc.GetTimeStamp();
  const ND::TVldRange& vr = this->GetVldRange();

  return
     (       task             == this->GetTask()
        && ( vc.GetDetector() &  vr.GetDetectorMask() )
        && ( vc.GetSimFlag()  &  vr.GetSimMask() )
        && (    ts            >= vr.GetTimeEnd()
             || ts            <  vr.GetTimeStart() )
     );

}

//.....................................................................


Bool_t ND::TDbiValidityRec::IsCompatible(const ND::TVldContext& vc,
                                    const TDbi::Task& task) const {
//
//
//  Purpose: See if result is compatible with query.
//
//  Arguments:
//    vc           in    Context of new query
//    task         in    Task of new query
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o See if result is compatible with query.

//  Program Notes:-
//  =============

//  None.

   DbiDebug(  " ND::TDbiValidityRec::IsCompatible : tasks:"
     << task << "," << fTask
     << " is compat: " << fVldRange.IsCompatible(vc) << "  "
     << "   range " << fVldRange.AsString() << "  "
     << "   context " << vc.AsString() << "  ");

  return task == fTask  && fVldRange.IsCompatible(vc);

}

//.....................................................................


Bool_t ND::TDbiValidityRec::IsHigherPriority(const ND::TDbiValidityRec& that,
					Bool_t useCreationDate) const {
//
//  Purpose:  Compare priorities (used in context queries to resolve ambiguities between VLD entries).
//
//  Arguments:
//    that             in    The ND::TDbiValidityRec to be compared with
//    useCreationDate  in    If false (the default value) use the T2K resolution scheme
//                           which is resolved, in order, by EPOCH, STARTTIME, INSERTDATE
//                           If true use the MINOS resolution scheme which is resolved by CREATIONDATE.
//
//  Return:   True if this is of higher priority than that.

  if ( useCreationDate) return fCreationDate > that.fCreationDate;

  if ( fEpoch != that.fEpoch ) return fEpoch > that.fEpoch;
  if ( fVldRange.GetTimeStart() != that.fVldRange.GetTimeStart() )
    return fVldRange.GetTimeStart() > that.fVldRange.GetTimeStart();
  return fInsertDate > that.fInsertDate;

}

//.....................................................................


void ND::TDbiValidityRec::SetTimeWindow(const ND::TVldTimeStamp& start,
                                   const ND::TVldTimeStamp& end) {
//
//
//  Purpose:  Set supplied start and end times.
//
//  Arguments:
//    start        in    Start time to be set,
//    end          in    End time to be set.
//
//  Return:   None.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Set time windown on current record.

//  Program Notes:-
//  =============

//  The method here is rather clumsy, but I don't know of anything
//  better!

  ND::TVldRange  range(fVldRange.GetDetectorMask(),
                  fVldRange.GetSimMask(),
                  start,
                  end,
                  fVldRange.GetDataSource() );
  fVldRange = range;

}

//.....................................................................

void ND::TDbiValidityRec::Store(ND::TDbiOutRowStream& ors,
                           const ND::TDbiValidityRec* /* vrec */) const {
//
//
//  Purpose:  Stream object to output row stream
//
//  Arguments:
//    ors          in    Output row stream.
//    vrec         in    =0.  If filling other table rows it points
//                       to the associated validity record.
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o  Stream object to output row stream.

//  Program Notes:-
//  =============

//  None.

//  Write data to row.

    ors << fSeqNo
        << fVldRange.GetTimeStart()
        << fVldRange.GetTimeEnd();
    if ( ors.HasEpoch() ) ors << fEpoch << fReality;
    ors << fVldRange.GetDetectorMask()
        << fVldRange.GetSimMask()
        << fTask
        << fAggregateNo
        << fCreationDate
        << fInsertDate;
}
//.....................................................................

void ND::TDbiValidityRec::Streamer(ND::TDbiBinaryFile& file) {
//
//
//  Purpose:  I/O to binary file
//
//  Program Notes:-
//  =============

//  Don't store the pointer fTableProxy (don't want to get into storing
//  pointers!); the ND::TDbiCache will fix it up on input

  if ( file.IsReading() ) {
    file >> fAggregateNo
         >> fCreationDate
	 >> fEpoch
	 >> fReality
         >> fDbNo
         >> fInsertDate
         >> fIsGap
         >> fTask
         >> fSeqNo
         >> fVldRange;
    fTableProxy = 0;
  }
  else if ( file.IsWriting() ) {
    file << fAggregateNo
         << fCreationDate
	 << fEpoch
	 << fReality
         << fDbNo
         << fInsertDate
         << fIsGap
         << fTask
         << fSeqNo
         << fVldRange;
  }
}

//.....................................................................

void ND::TDbiValidityRec::Trim(const ND::TVldTimeStamp& queryTime,
                          const ND::TDbiValidityRec& other) {
//
//
//  Purpose:  Trim this validity record so that represents
//            best validity record for query.
//
//  Arguments:
//    queryTime    in    Time of query
//    other        in    ND::TDbiValidity record satisfying query
//
//  Return:    None.
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o Update this validity record so that it remains the best
//    validity record taking into account the supplied record.


//  Program Notes:-
//  =============

//  This is the function that deal with validity management.
//  It takes into account that several validity records may
//  overlap and that the best one is the one with:-
//    For MINOS:  the latest creation date
//    For T2K:    the highest EPOCH,TIMESTART,INSERTDATE
//  that brackets the query date.  Other higher priority entries
//  may trim start or end times.

//  Assumptions:-
//  ===========
//
//  That entries are submitted in strict descending priority i.e.:-
//
//  1)  Entries for a higher priority database precede those from a
//      lower priority one.
//
//  2)  Within a database entries are in descending CREATIONDATE or
//      EPOCH,TIMESTART,INSERTDATE order.
//
//  This means that, until an entry that overlaps the supplied query time
//  is reach, the entries just restrict the possible time window.  The first
//  entry that overlaps the supplied time is accepted with a trimmed window
//  and all subsequent entries ignored.

//  Ignore other records that are either gaps or have wrong
//  aggregate number.

  if ( fAggregateNo != other.fAggregateNo || other.IsGap() ) return;

//  If this record is not a gap then the other record can be ignore
//  as it is of lower priority.

  if ( ! IsGap() ) return;

  ND::TVldTimeStamp start      = fVldRange.GetTimeStart();
  ND::TVldTimeStamp end        = fVldRange.GetTimeEnd();
  ND::TVldTimeStamp startOther = other.GetVldRange().GetTimeStart();
  ND::TVldTimeStamp endOther   = other.GetVldRange().GetTimeEnd();

// If entry brackets query date, then use it but with a validity that
// is trimmed by the current record.

  if ( startOther <= queryTime && endOther > queryTime ) {
    if ( start < startOther ) start = startOther;
    if ( end   > endOther   ) end   = endOther;
    *this = other;
    SetTimeWindow(start,end);
  }

// It doesn't bracket, so use it to trim the window

  else {

    if ( endOther <= queryTime ) {
      if ( start < endOther   ) SetTimeWindow(endOther,end);
    }
    else if ( startOther > queryTime ) {
      if ( end   > startOther ) SetTimeWindow(start, startOther);
    }
  }

}

/*    Template for New Member Function

//.....................................................................

ND::TDbiValidityRec:: {
//
//
//  Purpose:
//
//  Arguments:
//    xxxxxxxxx    in    yyyyyy
//
//  Return:
//
//  Contact:   N. West
//
//  Specification:-
//  =============
//
//  o

//  Program Notes:-
//  =============

//  None.


}

*/


