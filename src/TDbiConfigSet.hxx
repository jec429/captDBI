#ifndef DBICONFIGSET_H
#define DBICONFIGSET_H

/**
 *
 * $Id: TDbiConfigSet.hxx,v 1.1 2011/01/18 05:49:19 finch Exp $
 *
 * \class CP::TDbiConfigSet
 *
 *
 * \brief
 * <b>Concept</b>  A concrete data type corresponding to a single row in a
 *   database table of configuration data.
 *
 * \brief
 * <b>Purpose</b> Table row proxy for all Config data.
 *
 * Contact: A.Finch@lancaster.ac.uk
 *
 *
 */

#include "TDbiResultSetHandle.hxx"    //Needed for LinkDef
#include "TDbiWriter.hxx"    //Needed for LinkDef

#include "TDbiTableRow.hxx"
#include "TDbiFieldType.hxx"

#include <iosfwd>
#include <string>
#include <vector>

namespace CP {
    class TDbiConfigSet;
    class TDbiValidityRec;
    std::ostream& operator<<(std::ostream& s, const CP::TDbiConfigSet& cfSet);
}


namespace CP {
    class TDbiConfigSet : public TDbiTableRow {

    public:

// Constructors and destructors.
        virtual ~TDbiConfigSet();

// State testing member functions

        virtual TDbiTableRow* CreateTableRow() const {
            return new TDbiConfigSet;
        }
        Int_t GetAggregateNo() const {
            return fAggregateNo;
        }
        UInt_t GetNumParams() const {
            return fParams.size();
        }
        std::string GetParamName(UInt_t parNo) const;
        TDbiFieldType GetParamType(UInt_t parNo) const;
        std::string GetParamValue(UInt_t parNo) const;

// State changing member functions
        void Clear(const Option_t* = "") {
            fParams.clear();
        }
        void PushBack(const std::string& name,
                      const std::string& value,
                      const TDbiFieldType& type);
        void SetAggregateNo(Int_t aggNo) {
            fAggregateNo = aggNo;
        }

// I/O  member functions
        virtual void Fill(TDbiInRowStream& rs,
                          const TDbiValidityRec* vrec);
        virtual void Store(TDbiOutRowStream& ors,
                           const TDbiValidityRec* vrec) const;

    private:
// Constructors and destructors.


// Internal structures.
/// Internal structure used by TDbiCfgDialog to store Name/Value/Type triplets
        struct Param {
            Param() {  }
            Param(const Param& that) {
                *this = that;
            }
            Param(const std::string& name,
                  const std::string& value,
                  const TDbiFieldType& type) : Name(name), Value(value), Type(type) {
            }
            ~Param() {  }

            std::string Name;
            std::string Value;
            TDbiFieldType Type;
        };

// Data members

// Set of parameter, one per column.
        std::vector<Param*> fParams;

/// Aggregate number or:-
///     -1 Non-aggregated data or multiple aggregates
///     -2 undefined aggregates
        Int_t fAggregateNo;

        ClassDef(TDbiConfigSet,0)    // Configuration data.

    };
};


#endif  // DBICONFIGSET_H

