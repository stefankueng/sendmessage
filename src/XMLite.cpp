// XMLite.cpp: implementation of the XMLite class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMLite.h"
#include "auto_buffer.h"
#include <iostream>
#include <sstream>
#include <string>

static const TCHAR chXMLTagOpen     = '<';
static const TCHAR chXMLTagClose    = '>';
static const TCHAR chXMLTagPre  = '/';
static const TCHAR chXMLEscape = '\\';  // for value field escape

static const TCHAR szXMLPIOpen[] = _T("<?");
static const TCHAR szXMLPIClose[] = _T("?>");
static const TCHAR szXMLCommentOpen[] = _T("<!--");
static const TCHAR szXMLCommentClose[] = _T("-->");
static const TCHAR szXMLCDATAOpen[] = _T("<![CDATA[");
static const TCHAR szXMLCDATAClose[] = _T("]]>");

static const XENTITY x_EntityTable[] = {
        { '&', _T("&amp;"), 5 } ,
        { '\"', _T("&quot;"), 6 } ,
        { '\'', _T("&apos;"), 6 } ,
        { '<', _T("&lt;"), 4 } ,
        { '>', _T("&gt;"), 4 }
    };

PARSEINFO piDefault;
DISP_OPT optDefault;
XENTITYS entityDefault((LPXENTITY)x_EntityTable, sizeof(x_EntityTable)/sizeof(x_EntityTable[0]) );

LPTSTR _tcschrs( LPCTSTR psz, LPCTSTR pszchs )
{
    while( psz && *psz )
    {
        if( _tcschr( pszchs, *psz ) )
            return (LPTSTR)psz;
        psz++;
    }
    return NULL;
}

LPTSTR _tcsskip( LPCTSTR psz )
{
    //while( psz && *psz == ' ' && *psz == 13 && *psz == 10 ) psz++;
    while( psz && isspace(*psz) ) psz++;

    return (LPTSTR)psz;
}

LPTSTR _tcsechr( LPCTSTR psz, int ch, int escape )
{
    LPTSTR pch = (LPTSTR)psz;

    while( pch && *pch )
    {
        if( escape != 0 && *pch == escape )
            pch++;
        else
        if( *pch == ch )
            return (LPTSTR)pch;
        pch++;
    }
    return pch;
}

int _tcselen( int escape, LPTSTR srt, LPTSTR end = NULL )
{
    int len = 0;
    LPTSTR pch = srt;
    if( end==NULL ) end = (LPTSTR)sizeof(long);
    LPTSTR prev_escape = NULL;
    while( pch && *pch && pch<end )
    {
        if( escape != 0 && *pch == escape && prev_escape == NULL )
            prev_escape = pch;
        else
        {
            prev_escape = NULL;
            len++;
        }
        pch++;
    }
    return len;
}

void _tcsecpy( LPTSTR psz, int escape, LPTSTR srt, LPTSTR end = NULL )
{
    LPTSTR pch = srt;
    if( end==NULL ) end = (LPTSTR)sizeof(long);
    LPTSTR prev_escape = NULL;
    while( pch && *pch && pch<end )
    {
        if( escape != 0 && *pch == escape && prev_escape == NULL )
            prev_escape = pch;
        else
        {
            prev_escape = NULL;
            *psz++ = *pch;
        }

        pch++;
    }

    *psz = '\0';
}

LPTSTR _tcsepbrk( LPCTSTR psz, LPCTSTR chset, int escape )
{
    LPTSTR pch = (LPTSTR)psz;
    LPTSTR prev_escape = NULL;
    while( pch && *pch )
    {
        if( escape != 0 && *pch == escape && prev_escape == NULL )
            prev_escape = pch;
        else
        {
            prev_escape = NULL;
            if( _tcschr( chset, *pch ) )
                return (LPTSTR)pch;
        }
        pch++;
    }
    return pch;
}

int _tcsenicmp( LPCTSTR psz, LPCTSTR str, int len, int escape )
{
    LPTSTR pch = (LPTSTR)psz;
    LPTSTR prev_escape = NULL;
    LPTSTR des = (LPTSTR)str;
    int i = 0;

    while( pch && *pch && i < len )
    {
        if( escape != 0 && *pch == escape && prev_escape == NULL )
            prev_escape = pch;
        else
        {
            prev_escape = NULL;
            if( tolower(*pch) != tolower(des[i]) )
                break;
            i++;
        }
        pch ++;
    }

    // find
    if( i == len )
        return 0;
    if( psz[i] > des[i] )
        return 1;
    return -1;
}

LPTSTR _tcsenistr( LPCTSTR psz, LPCTSTR str, int len, int escape )
{
    LPTSTR pch = (LPTSTR)psz;
    LPTSTR prev_escape = NULL;

    while( pch && *pch )
    {
        if( escape != 0 && *pch == escape && prev_escape == NULL )
            prev_escape = pch;
        else
        {
            prev_escape = NULL;
            if( _tcsenicmp( pch, str, len, escape ) == 0 )
                return (LPTSTR)pch;
        }
        pch++;
    }
    return pch;
}

LPTSTR _tcseistr( LPCTSTR psz, LPCTSTR str, int escape )
{
    int len = _tcslen( str );
    return _tcsenistr( psz, str, len, escape );
}

void _SetString( LPTSTR psz, LPTSTR end, std::wstring* ps, bool trim = FALSE, int escape = 0 )
{
    //trim
    if( trim )
    {
        while( psz && psz < end && _istspace(*psz) ) psz++;
        while( (end-1) && psz < (end-1) && _istspace(*(end-1)) ) end--;
    }
    int len = end - psz;
    if( len <= 0 ) return;
    if( escape )
    {
        len = _tcselen( escape, psz, end );
        auto_buffer<TCHAR> pss (len + 1);
        _tcsecpy( pss, escape, psz, end );
        ps->assign(pss);
    }
    else
    {
        auto_buffer<TCHAR> pss (len + 1);
        memcpy( pss, psz, len*sizeof(TCHAR) );
        pss[len] = '\0';
        ps->assign(pss);
    }
}

_tagXMLNode::~_tagXMLNode()
{
    Close();
}

void _tagXMLNode::Close()
{
    for( size_t i = 0 ; i < childs.size(); i ++)
    {
        LPXNode p = childs[i];
        if( p )
        {
            delete p; childs[i] = NULL;
        }
    }
    childs.clear();

    for( size_t i = 0 ; i < attrs.size(); i ++)
    {
        LPXAttr p = attrs[i];
        if( p )
        {
            delete p; attrs[i] = NULL;
        }
    }
    attrs.clear();
}

LPTSTR _tagXMLNode::LoadAttributes( LPCTSTR pszAttrs , LPPARSEINFO pi /*= &piDefault*/)
{
    LPTSTR xml = (LPTSTR)pszAttrs;

    while( xml && *xml )
    {
        if(( xml = _tcsskip( xml ) ) != NULL)
        {
            // close tag
            if( *xml == chXMLTagClose || *xml == chXMLTagPre )
                // wel-formed tag
                return xml;

            // XML Attr Name
            TCHAR* pEnd = _tcspbrk( xml, _T(" =") );
            if( pEnd == NULL )
            {
                // error
                if( pi->erorr_occur == false )
                {
                    pi->erorr_occur = true;
                    pi->error_pointer = xml;
                    pi->error_code = PIE_ATTR_NO_VALUE;

                    auto_buffer<TCHAR> sbuf (100 + name.size());
                    _stprintf_s(sbuf, 100 + name.size(), _T("<%s> attribute has error "), name.c_str());
                    pi->error_string = sbuf;
                }
                return NULL;
            }

            LPXAttr attr = new XAttr;
            attr->parent = this;

            // XML Attr Name
            _SetString( xml, pEnd, &attr->name );

            // add new attribute
            attrs.push_back( attr );
            xml = pEnd;

            // XML Attr Value
            if( ( xml = _tcsskip( xml ) ) != NULL )
            {
                //if( xml = _tcschr( xml, '=' ) )
                if( *xml == '=' )
                {
                    if( ( xml = _tcsskip( ++xml ) ) != NULL )
                    {
                        // if " or '
                        // or none quote
                        int quote = *xml;
                        if( quote == '"' || quote == '\'' )
                            pEnd = _tcsechr( ++xml, quote, chXMLEscape );
                        else
                        {
                            //attr= value>
                            // none quote mode
                            //pEnd = _tcsechr( xml, ' ', '\\' );
                            pEnd = _tcsepbrk( xml, _T(" >"), chXMLEscape );
                        }

                        bool trim = pi->trim_value;
                        TCHAR escape = pi->escape_value;
                        //_SetString( xml, pEnd, &attr->value, trim, chXMLEscape );
                        _SetString( xml, pEnd, &attr->value, trim, escape );
                        xml = pEnd;
                        // ATTRVALUE
                        if( pi->entity_value && pi->entitys )
                            attr->value = pi->entitys->Ref2Entity(attr->value.c_str());

                        if( quote == '"' || quote == '\'' )
                            xml++;
                    }
                }
            }
        }
    }

    // not wel-formed tag
    return NULL;
}

LPTSTR _tagXMLNode::LoadAttributes( LPCTSTR pszAttrs, LPCTSTR pszEnd, LPPARSEINFO pi /*= &piDefault*/ )
{
    LPTSTR xml = (LPTSTR)pszAttrs;

    while( xml && *xml )
    {
        if( ( xml = _tcsskip( xml ) ) != NULL )
        {
            // close tag
            if( xml >= pszEnd )
                // wel-formed tag
                return xml;

            // XML Attr Name
            TCHAR* pEnd = _tcspbrk( xml, _T(" =") );
            if( pEnd == NULL )
            {
                // error
                if( pi->erorr_occur == false )
                {
                    pi->erorr_occur = true;
                    pi->error_pointer = xml;
                    pi->error_code = PIE_ATTR_NO_VALUE;
                    auto_buffer<TCHAR> sbuf (100 + name.size());
                    _stprintf_s(sbuf, 100 + name.size(), _T("<%s> attribute has error "), name.c_str());
                    pi->error_string = sbuf;
                }
                return NULL;
            }

            LPXAttr attr = new XAttr;
            attr->parent = this;

            // XML Attr Name
            _SetString( xml, pEnd, &attr->name );

            // add new attribute
            attrs.push_back( attr );
            xml = pEnd;

            // XML Attr Value
            if( ( xml = _tcsskip( xml ) ) != NULL )
            {
                //if( xml = _tcschr( xml, '=' ) )
                if( *xml == '=' )
                {
                    if( ( xml = _tcsskip( ++xml ) ) != NULL )
                    {
                        // if " or '
                        // or none quote
                        int quote = *xml;
                        if( quote == '"' || quote == '\'' )
                            pEnd = _tcsechr( ++xml, quote, chXMLEscape );
                        else
                        {
                            //attr= value>
                            // none quote mode
                            //pEnd = _tcsechr( xml, ' ', '\\' );
                            pEnd = _tcsepbrk( xml, _T(" >"), chXMLEscape );
                        }

                        bool trim = pi->trim_value;
                        TCHAR escape = pi->escape_value;
                        //_SetString( xml, pEnd, &attr->value, trim, chXMLEscape );
                        _SetString( xml, pEnd, &attr->value, trim, escape );
                        xml = pEnd;
                        // ATTRVALUE
                        if( pi->entity_value && pi->entitys )
                            attr->value = pi->entitys->Ref2Entity(attr->value.c_str());

                        if( quote == '"' || quote == '\'' )
                            xml++;
                    }
                }
            }
        }
    }

    // not wel-formed tag
    return NULL;
}

LPTSTR _tagXMLNode::LoadProcessingInstrunction( LPCTSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
    // find the end of pi
    LPTSTR end = _tcsenistr( pszXml, szXMLPIClose, sizeof(szXMLPIClose)-1, pi ? pi->escape_value : 0 );
    if( end == NULL )
        return NULL;

    // process pi
    if( doc )
    {
        LPTSTR xml = (LPTSTR)pszXml;

        LPXNode node = new XNode;
        node->parent = this;
        node->doc = doc;
        node->type = XNODE_PI;

        xml += sizeof(szXMLPIOpen)-1;
        TCHAR* pTagEnd = _tcspbrk( xml, _T(" ?>") );
        _SetString( xml, pTagEnd, &node->name );
        xml = pTagEnd;

        node->LoadAttributes( xml, end, pi );

        doc->childs.push_back( node );
    }

    end += sizeof(szXMLPIClose)-1;
    return end;
}

LPTSTR _tagXMLNode::LoadComment( LPCTSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
    // find the end of comment
    LPTSTR end = _tcsenistr( pszXml, szXMLCommentClose, sizeof(szXMLCommentClose)-1, pi ? pi->escape_value : 0 );
    if( end == NULL )
        return NULL;

    // process comment
    LPXNode par = parent;
    if( parent == NULL && doc )
        par = (LPXNode)&doc;
    if( par )
    {
        LPTSTR xml = (LPTSTR)pszXml;
        xml += sizeof(szXMLCommentOpen)-1;

        LPXNode node = new XNode;
        node->parent = this;
        node->doc = doc;
        node->type = XNODE_COMMENT;
        node->name = _T("#COMMENT");
        _SetString( xml, end, &node->value, FALSE );

        par->childs.push_back( node );
    }

    end += sizeof(szXMLCommentClose)-1;
    return end;
}

LPTSTR _tagXMLNode::LoadCDATA( LPCTSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
    // find the end of CDATA
    LPTSTR end = _tcsenistr( pszXml, szXMLCDATAClose, sizeof(szXMLCDATAClose)-1, pi ? pi->escape_value : 0 );
    if( end == NULL )
        return NULL;

    // process CDATA
    LPXNode par = parent;
    if( parent == NULL && doc )
        par = (LPXNode)&doc;
    if( par )
    {
        LPTSTR xml = (LPTSTR)pszXml;
        xml += sizeof(szXMLCDATAOpen)-1;

        LPXNode node = new XNode;
        node->parent = this;
        node->doc = doc;
        node->type = XNODE_CDATA;
        node->name = _T("#CDATA");
        _SetString( xml, end, &node->value, FALSE );

        par->childs.push_back( node );
    }

    end += sizeof(szXMLCDATAClose)-1;
    return end;
}

LPTSTR LoadOtherNodes( LPXNode node, bool* pbRet, LPCTSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
    LPTSTR xml = (LPTSTR)pszXml;
    bool do_other_type = true;
    *pbRet = false;

    while( xml && do_other_type )
    {
        do_other_type = false;

        xml = _tcsskip( xml );
        LPTSTR prev = xml;
        // is PI( Processing Instruction ) Node?
        if( _tcsnicmp( xml, szXMLPIOpen, sizeof(szXMLPIOpen)-1 ) == 0 )
        {
            // processing instrunction parse
            // return pointer is next node of pi
            xml = node->LoadProcessingInstrunction( xml, pi );
            //if( xml == NULL )
            //  return NULL;
            // restart xml parse
        }

        if( xml != prev )
            do_other_type = true;
        xml = _tcsskip( xml );
        prev = xml;

        // is comment Node?
        if( _tcsnicmp( xml, szXMLCommentOpen, sizeof(szXMLCommentOpen)-1 ) == 0 )
        {
            // processing comment parse
            // return pointer is next node of comment
            xml = node->LoadComment( xml, pi );
            // comment node is terminal node
            if( node->parent && node->parent->type != XNODE_DOC
                && xml != prev )
            {
                *pbRet = true;
                return xml;
            }
            // restart xml parse when this node is root doc node
        }

        if( xml != prev )
            do_other_type = true;

        xml = _tcsskip( xml );
        prev = xml;
        // is CDATA Node?
        if( _tcsnicmp( xml, szXMLCDATAOpen, sizeof(szXMLCDATAOpen)-1 ) == 0 )
        {
            // processing CDATA parse
            // return pointer is next node of CDATA
            xml = node->LoadCDATA( xml, pi );
            // CDATA node is terminal node
            if( node->parent && node->parent->type != XNODE_DOC
                && xml != prev )
            {
                *pbRet = true;
                return xml;
            }
            // restart xml parse when this node is root doc node
        }

        if( xml != prev )
            do_other_type = true;
    }

    return xml;
}

LPTSTR _tagXMLNode::Load( LPCTSTR pszXml, LPPARSEINFO pi /*= &piDefault*/ )
{
    // Close it
    Close();

    LPTSTR xml = (LPTSTR)pszXml;

    xml = _tcschr( xml, chXMLTagOpen );
    if( xml == NULL )
        return NULL;

    // Close Tag
    if( *(xml+1) == chXMLTagPre ) // </Close
        return xml;

    // Load Other Node before <Tag>(pi, comment, CDATA etc)
    bool bRet = false;
    LPTSTR ret = NULL;
    ret = LoadOtherNodes( this, &bRet, xml, pi );
    if( ret != NULL )
        xml = ret;
    if( bRet )
        return xml;

    // XML Node Tag Name Open
    xml++;
    TCHAR* pTagEnd = _tcspbrk( xml, _T(" />\t\r\n") );
    _SetString( xml, pTagEnd, &name );
    xml = pTagEnd;
    // Generate XML Attributte List
    if( ( xml = LoadAttributes( xml, pi ) ) != NULL )
    {
        // alone tag <TAG ... />
        if( *xml == chXMLTagPre )
        {
            xml++;
            if( *xml == chXMLTagClose )
                // wel-formed tag
                return ++xml;
            else
            {
                // error: <TAG ... / >
                if( pi->erorr_occur == false )
                {
                    pi->erorr_occur = true;
                    pi->error_pointer = xml;
                    pi->error_code = PIE_ALONE_NOT_CLOSED;
                    pi->error_string = _T("Element must be closed.");
                }
                // not wel-formed tag
                return NULL;
            }
        }
        else
        // open/close tag <TAG ..> ... </TAG>
        //                        ^- current pointer
        {
            // if text value is not exist, then assign value
            //if( this->value.IsEmpty() || this->value == _T("") )
            if( XIsEmptyString( value.c_str() ) )
            {
                // Text Value
                TCHAR* pEnd = _tcsechr( ++xml, chXMLTagOpen, chXMLEscape );
                if( pEnd == NULL )
                {
                    if( pi->erorr_occur == false )
                    {
                        pi->erorr_occur = true;
                        pi->error_pointer = xml;
                        pi->error_code = PIE_NOT_CLOSED;
                        auto_buffer<TCHAR> sbuf (100 + name.size() + value.size());
                        _stprintf_s(sbuf, 100 + name.size() + value.size(), _T("%s must be closed with </%s>"), name.c_str(), value.c_str());
                        pi->error_string = sbuf;
                    }
                    // error cos not exist CloseTag </TAG>
                    return NULL;
                }

                bool trim = pi->trim_value;
                TCHAR escape = pi->escape_value;
                //_SetString( xml, pEnd, &value, trim, chXMLEscape );
                _SetString( xml, pEnd, &value, trim, escape );

                xml = pEnd;
                // TEXTVALUE reference
                if( pi->entity_value && pi->entitys )
                    value = pi->entitys->Ref2Entity(value.c_str());
            }

            // generate child nodes
            while( xml && *xml )
            {
                LPXNode node = new XNode;
                node->parent = this;
                node->doc = doc;
                node->type = type;

                xml = node->Load( xml,pi );
                if( node->name.empty() == FALSE )
                {
                    childs.push_back( node );

                }
                else
                {
                    delete node;
                }

                // open/close tag <TAG ..> ... </TAG>
                //                             ^- current pointer
                // CloseTag case
                if( xml && *xml && *(xml+1) && *xml == chXMLTagOpen && *(xml+1) == chXMLTagPre )
                {
                    // </Close>
                    xml+=2; // C

                    if( ( xml = _tcsskip( xml ) ) != NULL )
                    {
                        std::wstring closename;
                        TCHAR* pEnd = _tcspbrk( xml, _T(" >") );
                        if( pEnd == NULL )
                        {
                            if( pi->erorr_occur == false )
                            {
                                pi->erorr_occur = true;
                                pi->error_pointer = xml;
                                pi->error_code = PIE_NOT_CLOSED;
                                auto_buffer<TCHAR> sbuf (100 + name.size() + value.size());
                                _stprintf_s(sbuf, 100 + name.size() + value.size(), _T("%s must be closed with </%s>"), name.c_str(), value.c_str());
                                pi->error_string = sbuf;
                            }
                            // error
                            return NULL;
                        }
                        _SetString( xml, pEnd, &closename );
                        if( closename == this->name )
                        {
                            // wel-formed open/close
                            xml = pEnd+1;
                            // return '>' or ' ' after pointer
                            return xml;
                        }
                        else
                        {
                            xml = pEnd+1;
                            // 2004.6.15 - example <B> alone tag
                            // now it can parse with attribute 'force_arse'
                            if( pi->force_parse == false )
                            {
                                // not welformed open/close
                                if( pi->erorr_occur == false )
                                {
                                    pi->erorr_occur = true;
                                    pi->error_pointer = xml;
                                    pi->error_code = PIE_NOT_NESTED;
                                    auto_buffer<TCHAR> sbuf (100 + name.size() + closename.size());
                                    _stprintf_s(sbuf, 100 + name.size(), _T("'<%s> ... </%s>' is not wel-formed."), name.c_str(), closename.c_str());
                                    pi->error_string = sbuf;
                                }
                                return NULL;
                            }
                        }
                    }
                }
                else    // Alone child Tag Loaded
                        // else 해야하는지 말아야하는지 의심간다.
                {

                    //if( xml && this->value.IsEmpty() && *xml !=chXMLTagOpen )
                    if( xml && XIsEmptyString( value.c_str() ) && *xml !=chXMLTagOpen )
                    {
                        // Text Value
                        TCHAR* pEnd = _tcsechr( xml, chXMLTagOpen, chXMLEscape );
                        if( pEnd == NULL )
                        {
                            // error cos not exist CloseTag </TAG>
                            if( pi->erorr_occur == false )
                            {
                                pi->erorr_occur = true;
                                pi->error_pointer = xml;
                                pi->error_code = PIE_NOT_CLOSED;
                                auto_buffer<TCHAR> sbuf (100 + name.size() + value.size());
                                _stprintf_s(sbuf, 100 + name.size() + value.size(), _T("%s must be closed with </%s>"), name.c_str(), value.c_str());
                                pi->error_string = sbuf;
                            }
                            return NULL;
                        }

                        bool trim = pi->trim_value;
                        TCHAR escape = pi->escape_value;
                        //_SetString( xml, pEnd, &value, trim, chXMLEscape );
                        _SetString( xml, pEnd, &value, trim, escape );

                        xml = pEnd;
                        //TEXTVALUE
                        if( pi->entity_value && pi->entitys )
                            value = pi->entitys->Ref2Entity(value.c_str());
                    }
                }
            }
        }
    }

    return xml;
}

LPTSTR _tagXMLDocument::Load( LPCTSTR pszXml, LPPARSEINFO pi /*= NULL*/ )
{
    LPXNode node = new XNode;
    node->parent = (LPXNode)this;
    node->type = XNODE_ELEMENT;
    node->doc = this;
    LPTSTR end;

    if( pi == NULL )
        pi = &parse_info;

    if( (end = node->Load( pszXml, pi )) == NULL )
    {
        delete node;
        return NULL;
    }

    childs.push_back( node );

    // Load Other Node after </Tag>(pi, comment, CDATA etc)
    LPTSTR ret;
    bool bRet = false;
    ret = LoadOtherNodes( node, &bRet, end, pi );
    if( ret != NULL )
        end = ret;

    return end;
}

LPXNode _tagXMLDocument::GetRoot()
{
    XNodes::iterator it = childs.begin();
    for( ; it != childs.end() ; ++(it) )
    {
        LPXNode node = *it;
        if( node->type == XNODE_ELEMENT )
            return node;
    }
    return NULL;
}

std::wstring _tagXMLAttr::GetXML( LPDISP_OPT opt /*= &optDefault*/ )
{
    std::wostringstream os;
    //os << (LPCTSTR)name << "='" << (LPCTSTR)value << "' ";

    os << (LPCTSTR)name.c_str() << _T("=") << (TCHAR)opt->value_quotation_mark
        << (LPCTSTR)(opt->reference_value&&opt->entitys?opt->entitys->Entity2Ref(value.c_str()).c_str():value.c_str())
        << (TCHAR)opt->value_quotation_mark << _T(" ");
    return os.str().c_str();
}

std::wstring _tagXMLNode::GetXML( LPDISP_OPT opt /*= &optDefault*/ )
{
    std::wostringstream os;

    // tab
    if( opt && opt->newline )
    {
        if( opt && opt->newline )
            os << "\r\n";
        for( int i = 0 ; i < opt->tab_base ; i++)
            os << '\t';
    }

    if( type == XNODE_DOC )
    {
        for( size_t i = 0 ; i < childs.size(); i++ )
            os << (LPCTSTR)childs[i]->GetXML( opt ).c_str();
        return os.str().c_str();
    }
    else
    if( type == XNODE_PI )
    {
        // <?TAG
        os << szXMLPIOpen << (LPCTSTR)name.c_str();
        // <?TAG Attr1="Val1"
        if( attrs.empty() == false ) os << ' ';
        for( size_t i = 0 ; i < attrs.size(); i++ )
        {
            os << (LPCTSTR)attrs[i]->GetXML(opt).c_str();
        }
        //?>
        os << szXMLPIClose;
        return os.str().c_str();
    }
    else
    if( type == XNODE_COMMENT )
    {
        // <--comment
        os << szXMLCommentOpen << (LPCTSTR)value.c_str();
        //-->
        os << szXMLCommentClose;
        return os.str().c_str();
    }
    else
    if( type == XNODE_CDATA )
    {
        // <--comment
        os << szXMLCDATAOpen << (LPCTSTR)value.c_str();
        //-->
        os << szXMLCDATAClose;
        return os.str().c_str();
    }

    // <TAG
    os << '<' << (LPCTSTR)name.c_str();

    // <TAG Attr1="Val1"
    if( attrs.empty() == false ) os << ' ';
    for( size_t i = 0 ; i < attrs.size(); i++ )
    {
        os << (LPCTSTR)attrs[i]->GetXML(opt).c_str();
    }

    if( childs.empty() && value.empty() )
    {
        // <TAG Attr1="Val1"/> alone tag
        os << "/>";
    }
    else
    {
        // <TAG Attr1="Val1"> and get child
        os << '>';
        if( opt && opt->newline && !childs.empty() )
        {
            opt->tab_base++;
        }

        for( size_t i = 0 ; i < childs.size(); i++ )
            os << (LPCTSTR)childs[i]->GetXML( opt ).c_str();

        // Text Value
        if( value != _T("") )
        {
            if( opt && opt->newline && !childs.empty() )
            {
                if( opt && opt->newline )
                    os << _T("\r\n");
                for( int i = 0 ; i < opt->tab_base ; i++)
                    os << '\t';
            }
            os << (LPCTSTR)(opt->reference_value&&opt->entitys?opt->entitys->Entity2Ref(value.c_str()).c_str():value.c_str());
        }

        // </TAG> CloseTag
        if( opt && opt->newline && !childs.empty() )
        {
            os << "\r\n";
            for( int i = 0 ; i < opt->tab_base-1 ; i++)
                os << '\t';
        }
        os << "</" << (LPCTSTR)name.c_str() << '>';

        if( opt && opt->newline )
        {
            if( !childs.empty() )
                opt->tab_base--;
        }
    }

    return os.str().c_str();
}

std::wstring _tagXMLNode::GetText( LPDISP_OPT opt /*= &optDefault*/ )
{
    std::wostringstream os;

    if( type == XNODE_DOC )
    {
        for( size_t i = 0 ; i < childs.size(); i++ )
            os << (LPCTSTR)childs[i]->GetText( opt ).c_str();
    }
    else
    if( type == XNODE_PI )
    {
        // no text
    }
    else
    if( type == XNODE_COMMENT )
    {
        // no text
    }
    else
    if( type == XNODE_CDATA )
    {
        os << (LPCTSTR)value.c_str();
    }
    else
    if( type == XNODE_ELEMENT )
    {
        if( childs.empty() && value.empty() )
        {
            // no text
        }
        else
        {
            // childs text
            for( size_t i = 0 ; i < childs.size(); i++ )
                os << (LPCTSTR)childs[i]->GetText().c_str();

            // Text Value
            os << (LPCTSTR)(opt->reference_value&&opt->entitys?opt->entitys->Entity2Ref(value.c_str()).c_str():value.c_str());
        }
    }

    return os.str().c_str();
}

LPXAttr _tagXMLNode::GetAttr( LPCTSTR attrname )
{
    for( size_t i = 0 ; i < attrs.size(); i++ )
    {
        LPXAttr attr = attrs[i];
        if( attr )
        {
            if( attr->name == attrname )
                return attr;
        }
    }
    return NULL;
}

XAttrs _tagXMLNode::GetAttrs( LPCTSTR name )
{
    XAttrs attrs;
    for( size_t i = 0 ; i < attrs.size(); i++ )
    {
        LPXAttr attr = attrs[i];
        if( attr )
        {
            if( attr->name == name )
                attrs.push_back( attr );
        }
    }
    return attrs;
}

LPCTSTR _tagXMLNode::GetAttrValue( LPCTSTR attrname )
{
    LPXAttr attr = GetAttr( attrname );
    return attr ? (LPCTSTR)attr->value.c_str() : NULL;
}

XNodes _tagXMLNode::GetChilds()
{
    return childs;
}

XNodes _tagXMLNode::GetChilds( LPCTSTR name )
{
    XNodes nodes;
    for( size_t i = 0 ; i < childs.size(); i++ )
    {
        LPXNode node = childs[i];
        if( node )
        {
            if( node->name == name )
                nodes.push_back( node );
        }
    }
    return nodes;
}

LPXNode _tagXMLNode::GetChild( int i )
{
    if( i >= 0 && i < (int)childs.size() )
        return childs[i];
    return NULL;
}

int _tagXMLNode::GetChildCount()
{
    return childs.size();
}

LPXNode _tagXMLNode::GetChild( LPCTSTR name )
{
    for( size_t i = 0 ; i < childs.size(); i++ )
    {
        LPXNode node = childs[i];
        if( node )
        {
            if( node->name == name )
                return node;
        }
    }
    return NULL;
}

LPCTSTR _tagXMLNode::GetChildValue( LPCTSTR name )
{
    LPXNode node = GetChild( name );
    return (node != NULL)? (LPCTSTR)node->value.c_str() : NULL;
}

std::wstring    _tagXMLNode::GetChildText( LPCTSTR name, LPDISP_OPT opt /*= &optDefault*/ )
{
    LPXNode node = GetChild( name );
    return (node != NULL)? node->GetText(opt) : _T("");
}

LPXAttr _tagXMLNode::GetChildAttr( LPCTSTR name, LPCTSTR attrname )
{
    LPXNode node = GetChild(name);
    return node ? node->GetAttr(attrname) : NULL;
}

LPCTSTR _tagXMLNode::GetChildAttrValue( LPCTSTR name, LPCTSTR attrname )
{
    LPXAttr attr = GetChildAttr( name, attrname );
    return attr ? (LPCTSTR)attr->value.c_str() : NULL;
}

LPXNode _tagXMLNode::Find( LPCTSTR name )
{
    XNodes::iterator it = childs.begin();
    for( ; it != childs.end(); ++(it))
    {
        LPXNode child = *it;
        if( child->name == name )
            return child;

        XNodes::iterator it = child->childs.begin();
        for( ; it != child->childs.end(); ++(it))
        {
            LPXNode find = child->Find( name );
            if( find != NULL )
                return find;
        }
    }

    return NULL;
}

XNodes::iterator _tagXMLNode::GetChildIterator( LPXNode node )
{
    XNodes::iterator it = childs.begin();
    for( ; it != childs.end() ; ++(it) )
    {
        if( *it == node )
            return it;
    }
    return childs.end();
}

LPXNode _tagXMLNode::AppendChild( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
    return AppendChild( CreateNode( name, value ) );
}

LPXNode _tagXMLNode::AppendChild( LPXNode node )
{
    node->parent = this;
    node->doc = doc;
    childs.push_back( node );
    return node;
}

bool _tagXMLNode::RemoveChild( LPXNode node )
{
    XNodes::iterator it = GetChildIterator( node );
    if( it != childs.end() )
    {
        delete *it;
        childs.erase( it );
        return true;
    }
    return false;
}

LPXAttr _tagXMLNode::GetAttr( int i )
{
    if( i >= 0 && i < (int)attrs.size() )
        return attrs[i];
    return NULL;
}

XAttrs::iterator _tagXMLNode::GetAttrIterator( LPXAttr attr )
{
    XAttrs::iterator it = attrs.begin();
    for( ; it != attrs.end() ; ++(it) )
    {
        if( *it == attr )
            return it;
    }
    return attrs.end();
}

LPXAttr _tagXMLNode::AppendAttr( LPXAttr attr )
{
    attr->parent = this;
    attrs.push_back( attr );
    return attr;
}

bool _tagXMLNode::RemoveAttr( LPXAttr attr )
{
    XAttrs::iterator it = GetAttrIterator( attr );
    if( it != attrs.end() )
    {
        delete *it;
        attrs.erase( it );
        return true;
    }
    return false;
}

LPXNode _tagXMLNode::CreateNode( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
    LPXNode node = new XNode;
    node->name = name;
    node->value = value;
    return node;
}

LPXAttr _tagXMLNode::CreateAttr( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
    LPXAttr attr = new XAttr;
    attr->name = name;
    attr->value = value;
    return attr;
}

LPXAttr _tagXMLNode::AppendAttr( LPCTSTR name /*= NULL*/, LPCTSTR value /*= NULL*/ )
{
    return AppendAttr( CreateAttr( name, value ) );
}

LPXNode _tagXMLNode::DetachChild( LPXNode node )
{
    XNodes::iterator it = GetChildIterator( node );
    if( it != childs.end() )
    {
        childs.erase( it );
        return node;
    }
    return NULL;
}

LPXAttr _tagXMLNode::DetachAttr( LPXAttr attr )
{
    XAttrs::iterator it = GetAttrIterator( attr );
    if( it != attrs.end() )
    {
        attrs.erase( it );
        return attr;
    }
    return NULL;
}

void _tagXMLNode::CopyNode( LPXNode node )
{
    Close();

    doc = node->doc;
    parent = node->parent;
    name = node->name;
    value = node->value;
    type = node->type;

    // copy attributes
    for( size_t i = 0 ; i < node->attrs.size(); i++)
    {
        LPXAttr attr = node->attrs[i];
        if( attr )
            AppendAttr( attr->name.c_str(), attr->value.c_str() );
    }
}

void _tagXMLNode::_CopyBranch( LPXNode node )
{
    CopyNode( node );

    for( size_t i = 0 ; i < node->childs.size(); i++)
    {
        LPXNode child = node->childs[i];
        if( child )
        {
            LPXNode mychild = new XNode;
            mychild->CopyNode( child );
            AppendChild( mychild );

            mychild->_CopyBranch( child );
        }
    }
}

LPXNode _tagXMLNode::AppendChildBranch( LPXNode node )
{
    LPXNode child = new XNode;
    child->CopyBranch( node );

    return AppendChild( child );
}

void _tagXMLNode::CopyBranch( LPXNode branch )
{
    Close();

    _CopyBranch( branch );
}


_tagXMLEntitys::_tagXMLEntitys( LPXENTITY entities, int count )
{
    for( int i = 0; i < count; i++)
        push_back( entities[i] );
}

LPXENTITY _tagXMLEntitys::GetEntity( int entity )
{
    for( size_t i = 0 ; i < size(); i ++ )
    {
        if( at(i).entity == entity )
            return LPXENTITY(&at(i));
    }
    return NULL;
}

LPXENTITY _tagXMLEntitys::GetEntity( LPTSTR entity )
{
    for( size_t i = 0 ; i < size(); i ++ )
    {
        LPTSTR ref = (LPTSTR)at(i).ref;
        LPTSTR ps = entity;
        while( ref && *ref )
            if( *ref++ != *ps++ )
                break;
        if( ref && !*ref )  // found!
            return LPXENTITY(&at(i));
    }
    return NULL;
}

int _tagXMLEntitys::GetEntityCount( LPCTSTR str )
{
    int nCount = 0;
    LPTSTR ps = (LPTSTR)str;
    while( ps && *ps )
        if( GetEntity( *ps++ ) ) nCount ++;
    return nCount;
}

int _tagXMLEntitys::Ref2Entity( LPCTSTR estr, LPTSTR str, int strlen )
{
    LPTSTR pes = (LPTSTR)estr;
    LPTSTR ps = str;
    LPTSTR ps_end = ps+strlen;
    while( pes && *pes && ps < ps_end )
    {
        LPXENTITY ent = GetEntity( pes );
        if( ent )
        {
            // copy entity meanning char
            *ps = ent->entity;
            pes += ent->ref_len;
        }
        else
            *ps = *pes++;   // default character copy
        ps++;
    }
    *ps = '\0';

    // total copied characters
    return ps-str;
}

int _tagXMLEntitys::Entity2Ref( LPCTSTR str, LPTSTR estr, int estrlen )
{
    LPTSTR ps = (LPTSTR)str;
    LPTSTR pes = (LPTSTR)estr;
    LPTSTR pes_end = pes+estrlen;
    while( ps && *ps && pes < pes_end )
    {
        LPXENTITY ent = GetEntity( *ps );
        if( ent )
        {
            // copy entity string
            LPTSTR ref = (LPTSTR)ent->ref;
            while( ref && *ref )
                *pes++ = *ref++;
        }
        else
            *pes++ = *ps;   // default character copy
        ps++;
    }
    *pes = '\0';

    // total copied characters
    return pes-estr;
}

std::wstring _tagXMLEntitys::Ref2Entity( LPCTSTR estr )
{
    std::wstring es;
    if( estr )
    {
        int len = _tcslen(estr);
        auto_buffer<TCHAR> esbuf (len + 1);
        if( esbuf )
        {
            Ref2Entity( estr, esbuf, len );
            es = esbuf;
        }
    }
    return es;
}

std::wstring _tagXMLEntitys::Entity2Ref( LPCTSTR str )
{
    std::wstring s;
    if( str )
    {
        int nEntityCount = GetEntityCount(str);
        if( nEntityCount == 0 )
            return std::wstring(str);
        int len = _tcslen(str) + nEntityCount*10 ;
        auto_buffer<TCHAR> sbuf (len + 1);
        if( sbuf )
        {
            Entity2Ref( str, sbuf, len );
            s = sbuf;
        }
    }
    return s;
}

std::wstring XRef2Entity( LPCTSTR estr )
{
    return entityDefault.Ref2Entity( estr );
}

std::wstring XEntity2Ref( LPCTSTR str )
{
    return entityDefault.Entity2Ref( str );
}