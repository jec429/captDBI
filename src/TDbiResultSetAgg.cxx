// $Id: TDbiResultSetAgg.cxx,v 1.3 2012/06/14 10:55:22 finch Exp $


#include <algorithm>
#include <map>
#include <vector>

#include "TDbiCache.hxx"
#include "TDbiBinaryFile.hxx"
#include "TDbiDBProxy.hxx"
#include "TDbiResultSetAgg.hxx"
#include "TDbiResultSetNonAgg.hxx"
#include "TDbiResultKey.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiTableRow.hxx"
#include "TDbiTimerManager.hxx"
#include "TDbiValidityRecBuilder.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.h>
using std::endl;
#include "TVldRange.hxx"
#include "UtilString.hxx"

ClassImp(CP::TDbiResultSetAgg)

typedef vector<const CP::TDbiResultSet*>::const_iterator ConstResultItr_t;


//   Definition of static data members
//   *********************************



//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//.....................................................................
///\verbatim
///
///  Purpose:  Default constructor
///
///  Arguments:
///      tableName    in     Name of table.
///      tableRow     in     Pointer to a sample tableRow object.
///                          May be null.
///      cache        in/out Pointer to a cache. May be null.
///      vrecBuilder  in     Pointer to validity record builder containing
///                          aggregated records from query. May be null
///      proxy         in    Pointer to database proxy. May be null.
///      sqlQualifiers in    Extended Context sql qualifiers
///
///  Return:    n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Create Result from CP::TDbiInRowStream generated by query.
///
///
///  Program Notes:-
///  =============
///
///  tableRow is just used to create new subclass CP::TDbiTableRow objects.
///\endverbatim
CP::TDbiResultSetAgg::TDbiResultSetAgg(const string& tableName,
                           const CP::TDbiTableRow* tableRow,
                           CP::TDbiCache* cache,
                           const CP::TDbiValidityRecBuilder* vrecBuilder,
                           const CP::TDbiDBProxy* proxy,
                           const string& sqlQualifiers) :
CP::TDbiResultSet(0,0,sqlQualifiers),
fSize(0)
{


    typedef std::map<UInt_t,UInt_t> seqToRow_t;

  DbiTrace( "Creating CP::TDbiResultSetAgg" << "  ");
  SetTableName(tableName);
  if ( ! tableRow || ! cache || ! vrecBuilder || ! proxy ) return;

// Unpack the extended context SQL qualifiers.
// Don't use StringTok - it eats null strings
// e.g. abc;;def gives 2 substrings.

  string::size_type loc  = sqlQualifiers.find(';');
  string::size_type loc2 = sqlQualifiers.find(';',loc+1);
  string sqlData  = string(sqlQualifiers,loc+1,loc2-loc-1);
  string fillOpts = string(sqlQualifiers,loc2+1);

//Loop over all rows looking to see if they are already in
//the cache, and if not, recording their associated sequence numbers

  vector<UInt_t> reqSeqNos;  // Sequence numbers required from DB.
  seqToRow_t seqToRow;       // Map SeqNo - > RowNo.
//  Set up a Default database number, it will be updated if anything
//  needs to be read from the database.
  UInt_t dbNo = 0;
  Int_t maxRowNo = vrecBuilder->GetNumValidityRec() - 1;

//Ignore the first entry from the validity rec builder, it will be
//for Agg No = -1, which should not be present for aggregated data.
  for ( Int_t rowNo = 1; rowNo <= maxRowNo; ++rowNo ) {
    const CP::TDbiValidityRec& vrecRow = vrecBuilder->GetValidityRec(rowNo);

//  If its already in the cache, then just connect it in.
    const CP::TDbiResultSet* res = cache->Search(vrecRow,sqlQualifiers);
    DbiVerbose( "Checking validity rec " << rowNo
			      << " " << vrecRow
			      << "SQL qual: " << sqlQualifiers
			      << " cache search: " << (void*) res << "  ");
    if ( res ) {
      fResults.push_back(res);
      res->Connect();
      fSize += res->GetNumRows();
    }

//  If its not in the cache, but represents a gap, then create an empty
//  CP::TDbiResultSet and add it to the cache.
    else if ( vrecRow.IsGap() ) {
      CP::TDbiResultSet* newRes = new CP::TDbiResultSetNonAgg(0, tableRow, &vrecRow);
      cache->Adopt(newRes,false);
      fResults.push_back(newRes);
      newRes->Connect();
    }

//  Neither in cache, nor a gap, so record its sequence number.
    else {
      UInt_t seqNo = vrecRow.GetSeqNo();
      reqSeqNos.push_back(seqNo);
      seqToRow[seqNo] = rowNo;
      fResults.push_back(0);
//    All data must come from a single database, so any vrec will
//    do to define which one.
      dbNo = vrecRow.GetDbNo();
    }
  }

//If there are required sequence numbers, then read them from the
//database and build CP::TDbiResultSets for each.

  if ( reqSeqNos.size() ) {
//  Sort into ascending order; it may simplify the query which will
//  block ranges of sequence numbers together.
    sort(reqSeqNos.begin(),reqSeqNos.end());
    CP::TDbiInRowStream* rs = proxy->QuerySeqNos(reqSeqNos,dbNo,sqlData,fillOpts);
//  Flag that data was read from Database.
    this->SetResultsFromDb();
    CP::TDbiTimerManager::gTimerManager.StartSubWatch(1);
    while ( ! rs->IsExhausted() ) {
      Int_t seqNo;
      *rs >> seqNo;
      rs->DecrementCurCol();
      Int_t rowNo = -2;
      if ( seqToRow.find(seqNo) == seqToRow.end() ) {
        DbiSevere(  "Unexpected SeqNo: " << seqNo << "  ");
      }
      else {
        rowNo = seqToRow[seqNo];
        DbiVerbose(  "Procesing SeqNo: " << seqNo
          << " for row " << rowNo << "  ");
      }

      const CP::TDbiValidityRec& vrecRow = vrecBuilder->GetValidityRec(rowNo);
      CP::TDbiResultSetNonAgg* newRes = new CP::TDbiResultSetNonAgg(rs,tableRow,&vrecRow);
//    Don't allow results from Extended Context queries to be reused.
      if ( this->IsExtendedContext() ) newRes->SetCanReuse(false);
      if ( rowNo == -2 ) {
        delete newRes;
      }
      else {
        DbiVerbose(  "SeqNo: " << seqNo
          << " produced " << newRes->GetNumRows() << " rows" << "  ");
//      Adopt but don't register key for this component, only the overall CP::TDbiResultSetAgg
//      will have a registered key.
        cache->Adopt(newRes,false);
        fResults[rowNo-1] = newRes;
        newRes->Connect();
        fSize += newRes->GetNumRows();
      }
    }

//  CP::TDbiInRowStream fully processed, so delete it.
    delete rs;
  }

//All component CP::TDbiResultSetNonAgg objects have now been located and
//connected in, so set up their access keys and determine the validty
//range by ANDing the time windows together.

  fRowKeys.reserve(fSize);

  CP::TDbiValidityRec vRec = vrecBuilder->GetValidityRec(1);
  for ( Int_t rowNo = 1; rowNo <= maxRowNo; ++rowNo ) {

    const CP::TDbiValidityRec& vrecRow = vrecBuilder->GetValidityRec(rowNo);
    CP::TVldRange r = vrecRow.GetVldRange();
    vRec.AndTimeWindow(r.GetTimeStart(),r.GetTimeEnd());

    const CP::TDbiResultSet* res = fResults[rowNo-1];
    if ( res ) {
      UInt_t numEnt = res->GetNumRows();
      for (UInt_t entNo = 0; entNo < numEnt; ++entNo )
         fRowKeys.push_back(res->GetTableRow(entNo));
    }
  }

// Now that the row look-up table has been built the natural index
// look-up table can be filled in.
  this->BuildLookUpTable();

// Set aggregate number to -1 to show that it has multiple aggregates
  vRec.SetAggregateNo(-1);
  SetValidityRec(vRec);

  DbiDebug(  "Aggregate contains " << fSize  << " entries.  vRec:-" << "  "
    << vRec << "  ");

   DbiInfo( "Created aggregated result set no. of rows: "
			     << this->GetNumRows() << "  ");

}

//.....................................................................
///\verbatim
///
///  Purpose: Destructor
///
///  Arguments:
///    None.
///
///  Return:    n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o  Destroy ResultAgg and disconnect all associated CP::TDbiResultSets,
///\endverbatim
CP::TDbiResultSetAgg::~TDbiResultSetAgg() {


//  Program Notes:-
//  =============

//  None.


  DbiTrace( "Destroying CP::TDbiResultSetAgg."  << "  ");

  for ( ConstResultItr_t itr = fResults.begin();
        itr != fResults.end();
        ++itr) if ( *itr ) (*itr)->Disconnect();

}
//.....................................................................

///
///  Purpose:  Create a key that corresponds to this result.
///            Caller must take ownership.
///
CP::TDbiResultKey* CP::TDbiResultSetAgg::CreateKey() const {

  CP::TDbiResultKey* key = 0;
  for ( ConstResultItr_t itr = fResults.begin();
        itr != fResults.end();
        ++itr ) {
    const CP::TDbiResultSet* result = *itr;
    if ( result ) {
      // Create key from first result.
      if ( ! key ) key = result->CreateKey();
      // Extend key from the remainder.
      else {
	const CP::TDbiValidityRec& vrec = result->GetValidityRec();
	key->AddVRecKey(vrec.GetSeqNo(),vrec.GetCreationDate());
      }
    }
  }

// Should not have an empty set, but just in case.
  if ( ! key ) key = new CP::TDbiResultKey();

  return key;

}
//.....................................................................
///\verbatim
///
///  Purpose:  Return TableRow from last query.
///
///  Arguments:
///    row         in    Required row.
///
///  Return:    TableRow ptr, or =0 if no row.
///\endverbatim
const CP::TDbiTableRow* CP::TDbiResultSetAgg::GetTableRow(UInt_t row) const {

//  Contact:   N. West

//  Program Notes:-
//  =============

//  None.

  return  ( row >= fRowKeys.size() ) ? 0 : fRowKeys[row];

}

//.....................................................................
///\verbatim
///
///  Purpose:  Get CP::TDbiValidityRec associated with table or row.
///
///  Arguments:
///    row          in    The table row
///                       or null (default) to get table CP::TDbiValidityRec
///  Program Notes:-
///  =============
///
///  The CP::TDbiValidityRec for the in-memory table is effectively an
///  AND of the CP::TDbiValidityRecs of the individual rows.
///
///\endverbatim
const CP::TDbiValidityRec& CP::TDbiResultSetAgg::GetValidityRec(
                                  const CP::TDbiTableRow* row) const {

//  Contact:   N. West
//


 if ( ! row ) return this->GetValidityRecGlobal();
 CP::TDbiResultSet* owner = row->GetOwner();
 return owner ? owner->GetValidityRecGlobal() : this->GetValidityRecGlobal();

}
//.....................................................................
///  Purpose:  Return true if result satisfies extended context query.
Bool_t CP::TDbiResultSetAgg::Satisfies(const string& sqlQualifiers)  {
//
//

//

  DbiDebug(  "Trying to satisfy: SQL: " << sqlQualifiers
    << "\n with CanReuse: " << this->CanReuse()
    << " sqlQualifiers: " << this->GetSqlQualifiers()
    << "  ");
 return    this->CanReuse()
        && this->GetSqlQualifiers() == sqlQualifiers;
}

//.....................................................................
///
///
///  Purpose:  I/O to binary file
///
void CP::TDbiResultSetAgg::Streamer(CP::TDbiBinaryFile& bf) {

//  Specification:-
//  =============

//  Output constituent non-gap CP::TDbiResultSetNonAgg objects.

  vector<const CP::TDbiResultSet*>::const_iterator itr = fResults.begin();
  vector<const CP::TDbiResultSet*>::const_iterator end = fResults.end();

  UInt_t numNonAgg = 0;
  for (; itr != end; ++itr) {
    const CP::TDbiResultSetNonAgg* rna = dynamic_cast<const CP::TDbiResultSetNonAgg*>(*itr);
    if ( rna && ! rna->GetValidityRecGlobal().IsGap() ) ++numNonAgg;
  }
  bf << numNonAgg;


  for (itr = fResults.begin(); itr != end; ++itr) {
    const CP::TDbiResultSetNonAgg* rna = dynamic_cast<const CP::TDbiResultSetNonAgg*>(*itr);
    if ( rna && ! rna->GetValidityRecGlobal().IsGap() ) bf << *rna;
  }
}

/*    Template for New Member Function

//.....................................................................

CP::TDbiResultSetAgg:: {
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


