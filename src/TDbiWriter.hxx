#ifndef DBIWRITER_H
#define DBIWRITER_H

/**
 *
 * $Id: TDbiWriter.hxx,v 1.1 2011/01/18 05:49:20 finch Exp $
 *
 * \class ND::TDbiWriter
 *
 *
 * \brief
 * <b>Concept</b>  Templated class ND::of pointers to Writer objects.
 *   Writer objects are lightweight and provide type safe write
 *   access to a single validity set of a specific table.
 *   A TDbiWriter knows about publishing protocols but uses a
 *   TDbiSqlValPacket both to hold the data in a type- neutral
 *   form and to do the actual I/O.
 *
 * \brief
 * <b>Purpose</b> Writers are the primary application output
 *   interface to the TDbi.  Users instantiate Writers with
 *   the information necessary define a validity set and then
 *   pass all the rows one at a time, to the writer.  The user
 *   is able to abort the process at any point before the final
 *   commit.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */
#include <list>
#include <string>

#include "TDbi.hxx"
#include "TDbiLogEntry.hxx"
#include "TDbiDatabaseManager.hxx"
#include "TVldRange.hxx"
#include "TVldTimeStamp.hxx"

namespace ND {
class TDbiSqlValPacket;
class TDbiTableProxy;
class TDbiValidityRec;
}

namespace ND {
template <class T> class TDbiWriter
{

public:

// Constructors and destructors.
           TDbiWriter();
           TDbiWriter(const ND::TVldRange& vr,
                     Int_t aggNo,
                     TDbi::Task task,
                     ND::TVldTimeStamp creationDate,
                     const std::string& dbName,
                     const std::string& logComment = "",
                     const std::string& tableName = "");
           TDbiWriter(const ND::TVldRange& vr,
                     Int_t aggNo,
                     TDbi::Task task = 0,
                     ND::TVldTimeStamp creationDate = ND::TVldTimeStamp(0,0),
                     UInt_t dbNo = 0,
                     const std::string& logComment = "",
                     const std::string& tableName = "");
           TDbiWriter(const TDbiValidityRec& vrec,
                     const std::string& dbName,
                     const std::string& logComment = "");
           TDbiWriter(const TDbiValidityRec& vrec,
                     UInt_t dbNo = 0,
                     const std::string& logComment = "");

  virtual ~TDbiWriter();

// State testing member functions

       UInt_t GetEpoch() const;
       TDbiTableProxy& TableProxy() const;

///    Open and ready to receive data.
          Bool_t IsOpen(Bool_t reportErrors = kTRUE) const;
///    Open and ready to receive/output data.
          Bool_t CanOutput(Bool_t reportErrors = kTRUE) const;

// State changing member functions

    void SetDbNo(UInt_t dbNo) { fDbNo = dbNo;}
    void SetDbName(const string& dbName);
    void SetEpoch(UInt_t epoch);
    void SetLogComment(const std::string& reason);
    // For setting of requireGlobal see TDbiCascader::AllocateSeqNo
    void SetRequireGlobalSeqno(Int_t requireGlobal) {fRequireGlobalSeqno = requireGlobal;}
    void SetOverlayCreationDate() {fUseOverlayCreationDate = kTRUE;}

//  I/O functions
  void Abort() { Reset(); }
  Bool_t Close(const char* fileSpec=0);
  Bool_t Open(const ND::TVldRange& vr,
            Int_t aggNo,
            TDbi::Task task,
            ND::TVldTimeStamp creationDate,
            const string& dbName,
            const std::string& logComment = "");
   Bool_t Open(const ND::TVldRange& vr,
            Int_t aggNo,
            TDbi::Task task = 0,
            ND::TVldTimeStamp creationDate = ND::TVldTimeStamp(),
            UInt_t dbNo = 0,
            const std::string& logComment = "");
 Bool_t Open(const TDbiValidityRec& vrec,
             const string& dbName,
             const std::string& logComment = "");
 Bool_t Open(const TDbiValidityRec& vrec,
              UInt_t dbNo = 0,
              const std::string& logComment = "");

  ND::TDbiWriter<T>& operator<<(const T& row);

private:

// State testing member functions

  Bool_t NeedsLogEntry() const;
  Bool_t WritingToMaster() const;

// State changing member functions

TDbiWriter(const TDbiWriter&); // Forbidden
TDbiWriter& operator=(const TDbiWriter&); // Forbidden

    void CompleteOpen(UInt_t dbNo = 0,
                      const std::string& logComment = "");
    void Reset();

static TDbiTableProxy& GetTableProxy();
static TDbiTableProxy& GetTableProxy(const std::string& tableName);

// Data members

/// Aggregate noumber for set.
  Int_t fAggregateNo;

///Database number in cascade
  UInt_t fDbNo;

/// The assembled record to be output. Never null.
  TDbiSqlValPacket* fPacket;

/// Controls SEQNO type (see TDbiCascader::AllocateSeqNo)
  Int_t fRequireGlobalSeqno;

/// Proxy to associated table.
  TDbiTableProxy* fTableProxy;

/// Associated table name.
  std::string fTableName;

/// Use overlay creation date if true.
  Bool_t fUseOverlayCreationDate;

/// Validity record. May be =0 if closed.
  TDbiValidityRec* fValidRec;

/// Associated log entry (if any) for update
  TDbiLogEntry fLogEntry;

ClassDefT(TDbiWriter<T>,0)          // Writer for specific database table.

};
};
ClassDefT2(TDbiWriter,T)

#endif  // DBIWRITER_H

