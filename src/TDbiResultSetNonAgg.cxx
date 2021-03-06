// $Id: TDbiResultSetNonAgg.cxx,v 1.3 2012/06/14 10:55:22 finch Exp $

#include "TDbiBinaryFile.hxx"
#include "TDbiResultKey.hxx"
#include "TDbiResultSetNonAgg.hxx"
#include "TDbiInRowStream.hxx"
#include "TDbiTableRow.hxx"
#include "TDbiTimerManager.hxx"
#include <TDbiLog.hxx>
#include <MsgFormat.hxx>

ClassImp(CP::TDbiResultSetNonAgg)

//   Definition of static data members
//   *********************************



//    Definition of all member functions (static or otherwise)
//    *******************************************************
//
//    -  ordered: ctors, dtor, operators then in alphabetical order.

//
//.....................................................................
///\verbatim
///  Purpose:  Default constructor
///
///  Arguments:
///      resultSet    in/out Pointer CP::TDbiInRowStream from query. May be null.
///      tableRow     in     Pointer to a sample tableRow object.
///                          May be null.
///      vrec         in     Pointer to validity record from query.
///                          May be null
///      dropSeqNo    in     If = kTRUE, drop SeqNo if it is the first col.
///      sqlQualifier in     Extended Context sql qualifiers
///
///  Return:    n/a
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Create Result from CP::TDbiInRowStream generated by query.  If first
///    column is named SeqNo then strip it off before filling each
///    CP::TDbiTableRow and quit as soon as the SeqNo changes.
///
///
///  Program Notes:-
///  =============
///
///  o tableRow is just used to create new subclass CP::TDbiTableRow objects.
///
///  o  The special treatment for tables that start with SeqNo allow
///     a single CP::TDbiInRowStream to fill multiple CP::TDbiResultSet objects but does
///     require that the result set is ordered by SeqNo.
///
///  The look-up table is not built by default, its construction is
///  triggered by use (GetTableRowByIndex).  For CP::TDbiResultSetNonAgg
///  that are part of a CP::TDbiResultSetAgg there is no need to build the
///  table.
///\endverbatim

CP::TDbiResultSetNonAgg::TDbiResultSetNonAgg(CP::TDbiInRowStream* resultSet,
                                             const CP::TDbiTableRow* tableRow,
                                             const CP::TDbiValidityRec* vrec,
                                             Bool_t dropSeqNo,
                                             const std::string& sqlQualifiers) :
    CP::TDbiResultSet(resultSet,vrec,sqlQualifiers),
    fBuffer(0) {

    DbiTrace("Start TDbiResultSetNonAgg");
    this->DebugCtor();

    if (!resultSet) {
        DbiTrace("NULL result set: Return without filling");
        return;
    }

    if (resultSet->IsExhausted()) {
        DbiTrace("Exhausted result set: Return without filling");
        return;
    }

    if (!tableRow) {
        DbiTrace("No table row: Return without filling");
        return;
    }

    if (vrec) {
        CP::TDbiTimerManager::gTimerManager.RecFillAgg(vrec->GetAggregateNo());
    }

    //Move to first row if result set not yet started.
    CP::TDbiInRowStream& rs = *resultSet;
    if (rs.IsBeforeFirst()) {
        DbiTrace("Fetch row");
        rs.FetchRow();
    }
    if (rs.IsExhausted()) {
        DbiTrace("Exhausted after fetch row");
        return;
    }

    //Check and load sequence number if necessary.
    Int_t seqNo = 0;
    if (dropSeqNo && rs.CurColName() == "SEQNO") {
        rs >> seqNo;
        rs.DecrementCurCol();
    }

    // Main (non-VLD) tables have a ROW_COUNTER (which has to be ignored when
    // reading).
    bool hasRowCounter = ! rs.IsVLDTable();

    // Create and fill table row object and move result set onto next row.
    while (! rs.IsExhausted()) {
        DbiTrace("Loop result stream " << seqNo);
        //  If stripping off sequence numbers check the next and quit,
        //  having restored the last, if it changes.
        if (seqNo != 0) {
            Int_t nextSeqNo;
            rs >> nextSeqNo;
            if (nextSeqNo != seqNo) {
                DbiTrace("Next seq number " << nextSeqNo);
                rs.DecrementCurCol();
                break;
            }
        }

        //  Strip off ROW_COUNTER if present.
        if (hasRowCounter) {
            rs.IncrementCurCol();
        }
        CP::TDbiTableRow* row = tableRow->CreateTableRow();
        if (vrec) {
            CP::TDbiTimerManager::gTimerManager.StartSubWatch(3);
        }
        row->SetOwner(this);
        DbiTrace("Fill user row class: " << vrec);
        row->Fill(rs,vrec);
        if (vrec) {
            CP::TDbiTimerManager::gTimerManager.StartSubWatch(2);
        }
        fRows.push_back(row);
        rs.FetchRow();
        if (vrec) {
            CP::TDbiTimerManager::gTimerManager.StartSubWatch(1);
        }
    }

    //Flag that data was read from Database.
    this->SetResultsFromDb();
    if (seqNo  == 0) {
        DbiInfo("Created unaggregated VLD result set no. of rows: "
                << this->GetNumRows() << "  ");
    }
    else  {
        DbiInfo("Created unaggregated result set for SeqNo: " << seqNo
                << " no. of rows: " << this->GetNumRows() << "  ");
    }
    
}


//.....................................................................
//
///\verbatim
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
///  o  Destroy ResultNonAgg and all its owned CP::TDbiTableRow subclass
///     objects.
///
///
///  Program Notes:-
///  =============
///
///  If fRows restored from BinaryFile then it doesn't own its
///  CP::TDbiTableRows.
///\endverbatim
CP::TDbiResultSetNonAgg::~TDbiResultSetNonAgg() {


    DbiTrace("Destroying CP::TDbiResultSetNonAgg."  << "  ");

    if (! fBuffer) for (std::vector<CP::TDbiTableRow*>::iterator itr = fRows.begin();
                            itr != fRows.end();
                            ++itr) {
            delete *itr;
        }
    else {
        delete [] fBuffer;
        fBuffer = 0;
    }
}
//.....................................................................
///  Purpose:  Create a key that corresponds to this result.
CP::TDbiResultKey* CP::TDbiResultSetNonAgg::CreateKey() const {
//
//



    std::string rowName("empty_table");
    const CP::TDbiTableRow* row = this->GetTableRow(0);
    if (row) {
        rowName = row->GetName();
    }
    const CP::TDbiValidityRec& vrec = this->GetValidityRec();
    return new CP::TDbiResultKey(this->TableName(),
                                 rowName,
                                 vrec.GetSeqNo(),
                                 vrec.GetCreationDate());

}

//.....................................................................

void CP::TDbiResultSetNonAgg::DebugCtor() const {

    DbiTrace("Creating CP::TDbiResultSetNonAgg " << (void*) this << "  ");
    static const CP::TDbiResultSetNonAgg* that = 0;
    if (this == that) {
        DbiError("Class not constructed");
    }
}

//.....................................................................
///\verbatim
///
///  Purpose:  Return TableRow from last query.
///
///  Arguments:
///    rowNum      in    Required row number (0..NumRows-1)
///
///  Return:    TableRow ptr, or =0 if no row.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o Return TableRow from last query, or =0 if no row.
///\endverbatim
const CP::TDbiTableRow* CP::TDbiResultSetNonAgg::GetTableRow(UInt_t rowNum) const {

//  Program Notes:-
//  =============

//  None.

    if (rowNum >= fRows.size()) {
        return 0;
    }
    return fRows[rowNum];
}

//.....................................................................
///\verbatim
///
///  Purpose: Return TableRow with supplied index, or =0 if no row.
///
///  Arguments:
///    index        in    Required index.
///
///  Return:    TableRow with required index, or =0 if no row.
///
///  Contact:   N. West
///
///  Specification:-
///  =============
///
///  o If look-up table not yet built, build it.
///
///  o Return TableRow with supplied index, or =0 if no row.
///\endverbatim
const CP::TDbiTableRow* CP::TDbiResultSetNonAgg::GetTableRowByIndex(UInt_t index) const {


    if (! this->LookUpBuilt()) {
        this->BuildLookUpTable();
    }

// The real look-up still takes place in the base class.
    return this->CP::TDbiResultSet::GetTableRowByIndex(index);

}
//.....................................................................
//
///\verbatim
///  Purpose:  Return true if owns row.
///
///
///  Program Notes:-
///  =============
///
///  Only CP::TDbiResultSetNonAggs own rows; the base class CP::TDbiResultSet supplies
///  the default method that returns false.
///\endverbatim
Bool_t CP::TDbiResultSetNonAgg::Owns(const CP::TDbiTableRow* row) const {

    std::vector<CP::TDbiTableRow*>::const_iterator itr    = fRows.begin();
    std::vector<CP::TDbiTableRow*>::const_iterator itrEnd = fRows.end();

    for (; itr != itrEnd; ++itr) if (*itr == row) {
            return kTRUE;
        }

    return kFALSE;


}

//.....................................................................
///  Purpose: Check to see if this Result matches the supplied  CP::TDbiValidityRec.
Bool_t CP::TDbiResultSetNonAgg::Satisfies(const CP::TDbiValidityRec& vrec,
                                          const std::string& sqlQualifiers) {
//
//


    DbiDebug("Trying to satisfy: Vrec " << vrec << " SQL: " << sqlQualifiers
             << "\n with CanReuse: " << this->CanReuse()
             << " vrec: " << this->GetValidityRec()
             << " sqlQualifiers: " << this->GetSqlQualifiers()
             << "  ");

    if (this->CanReuse()) {
        const CP::TDbiValidityRec& this_vrec = this->GetValidityRec();
        if (sqlQualifiers           == this->GetSqlQualifiers()
            && vrec.GetSeqNo()         == this_vrec.GetSeqNo()
            && vrec.GetCreationDate()  == this_vrec.GetCreationDate()
           ) {
            return kTRUE;
        }
    }

    return kFALSE;

}

//.....................................................................
///\verbatim
///
///  Purpose:  I/O to binary file
///
///  Program Notes:-
///  =============
///  Do I/O for base class CP::TDbiResultSet first.  Rebuild fIndexKeys on input.
///\endverbatim
void CP::TDbiResultSetNonAgg::Streamer(CP::TDbiBinaryFile& file) {

    if (file.IsReading()) {
        this->CP::TDbiResultSet::Streamer(file);
        DbiDebug("    Restoring CP::TDbiResultSetNonAgg ..." << "  ");
        file >> fRows;
//  Take ownership of the memory holding the array.
        fBuffer = file.ReleaseArrayBuffer();
        this->BuildLookUpTable();
        DbiDebug("    Restored CP::TDbiResultSetNonAgg. Size:"
                 << fRows.size() << " rows" << "  ");
    }
    else if (file.IsWriting()) {
        this->CP::TDbiResultSet::Streamer(file);
        DbiDebug("    Saving CP::TDbiResultSetNonAgg. Size:"
                 << fRows.size() << " rows" << "  ");
        file << fRows;
    }
}

/*    Template for New Member Function

//.....................................................................

CP::TDbiResultSetNonAgg:: {
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


