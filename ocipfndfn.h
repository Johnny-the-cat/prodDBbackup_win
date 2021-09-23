#pragma once
//Содержит определения типов для указателей функций

typedef sword(*pOCIEnvCreate)(OCIEnv        **hOraEnvp,
	ub4           mode,
	const void    *ctxp,
	const void    *(*malocfp)
	(void  *ctxp,
		size_t size),
	const void    *(*ralocfp)
	(void  *ctxp,
		void  *memptr,
		size_t newsize),
	const void(*mfreefp)
	(void  *ctxp,
		void  *memptr),
	size_t         xtramemsz,
	void           **usrmempp);

typedef sword (*pOCIEnvNlsCreate)(OCIEnv        **envhpp,
	ub4           mode,
	void         *ctxp,
	void         *(*malocfp)
	(void *ctxp,
		size_t size),
	void         *(*ralocfp)
	(void *ctxp,
		void *memptr,
		size_t newsize),
	void(*mfreefp)
	(void *ctxp,
		void *memptr),
	size_t        xtramemsz,
	void         **usrmempp,
	ub2           charset,
	ub2           ncharset );

typedef sword(*pOCITerminate)(ub4    mode);

typedef sword(*pOCIHandleAlloc)(const void *parenth,
	void **hndlpp,
	ub4 type,
	size_t xtramem_sz,
	void **usrmempp);

typedef sword(*pOCIServerAttach)(OCIServer     *srvhp,
	OCIError      *errhp,
	const OraText *dblink,
	sb4           dblink_len,
	ub4           mode);

typedef sword(*pOCIAttrSet)(void        *trgthndlp,
	ub4         trghndltyp,
	void        *attributep,
	ub4         size,
	ub4         attrtype,
	OCIError    *errhp);

typedef sword(*pOCISessionBegin)(OCISvcCtx     *svchp,
	OCIError      *errhp,
	OCISession    *usrhp,
	ub4           credt,
	ub4           mode);

typedef sword(*pOCISessionEnd)(OCISvcCtx       *svchp,
	OCIError        *errhp,
	OCISession      *usrhp,
	ub4             mode);

typedef sword(*pOCIStmtPrepare)(OCIStmt       *stmtp,
	OCIError      *errhp,
	const OraText *stmt,
	ub4           stmt_len,
	ub4           language,
	ub4           mode);

typedef sword(*pOCIDefineByPos)(OCIStmt     *stmtp,
	OCIDefine   **defnpp,
	OCIError    *errhp,
	ub4         position,
	void        *valuep,
	sb4         value_sz,
	ub2         dty,
	void        *indp,
	ub2         *rlenp,
	ub2         *rcodep,
	ub4         mode);

typedef sword(*pOCIStmtExecute)(OCISvcCtx           *svchp,
	OCIStmt             *stmtp,
	OCIError            *errhp,
	ub4                 iters,
	ub4                 rowoff,
	const OCISnapshot   *snap_in,
	OCISnapshot         *snap_out,
	ub4                 mode);

typedef sword(*pOCIBindByName)(OCIStmt       *stmtp,
	OCIBind       **bindpp,
	OCIError      *errhp,
	const OraText *placeholder,
	sb4           placeh_len,
	void          *valuep,
	sb4           value_sz,
	ub2           dty,
	void          *indp,
	ub2           *alenp,
	ub2           *rcodep,
	ub4           maxarr_len,
	ub4           *curelep,
	ub4           mode);

typedef sword(*pOCIBindByPos)(OCIStmt      *stmtp,
	OCIBind      **bindpp,
	OCIError     *errhp,
	ub4          position,
	void         *valuep,
	sb4          value_sz,
	ub2          dty,
	void         *indp,
	ub2          *alenp,
	ub2          *rcodep,
	ub4          maxarr_len,
	ub4          *curelep,
	ub4          mode);

typedef sword(*pOCITransCommit)(OCISvcCtx    *svchp,
	OCIError     *errhp,
	ub4           flags);

typedef sword(*pOCIHandleFree)(void      *hndlp,
	ub4       type);

typedef sword(*pOCIServerDetach)(OCIServer   *srvhp,
	OCIError    *errhp,
	ub4         mode);

typedef sword(*pOCIStmtFetch2)(OCIStmt     *stmthp,
	OCIError    *errhp,
	ub4         nrows,
	ub2         orientation,
	sb4         fetchOffset,
	ub4         mode);

typedef sword(*pOCIErrorGet)(void       *hndlp,
	ub4        recordno,
	OraText    *sqlstate,
	sb4        *errcodep,
	OraText    *bufp,
	ub4        bufsiz,
	ub4        type);

typedef sword(*pOCIRawAllocSize)(OCIEnv             *env,
	OCIError           *err,
	const OCIRaw       *raw,
	ub4                *allocsize);

typedef sword(*pOCIRawAssignBytes)(OCIEnv             *env,
	OCIError           *err,
	const ub1          *rhs,
	ub4                rhs_len,
	OCIRaw             **lhs);

typedef sword(*pOCILobFileSetName)(OCIEnv             *envhp,
	OCIError           *errhp,
	OCILobLocator      **filepp,
	CONST text         *dir_alias,
	ub2                d_length,
	CONST text         *filename,
	ub2                f_length);

typedef sword(*pOCIDescriptorAlloc)(const void    *parenth,
	void          **descpp,
	ub4           type,
	size_t        xtramem_sz,
	void          **usrmempp);

typedef sword(*pOCIDescriptorFree)(void     *descp,
	ub4      type);

typedef sword(*pOCILobFileOpen)(OCISvcCtx            *svchp,
	OCIError             *errhp,
	OCILobLocator        *filep,
	ub1                  mode);

typedef sword(*pOCILobFileClose)(OCISvcCtx            *svchp,
	OCIError             *errhp,
	OCILobLocator        *filep);

typedef sword(*pOCILobGetLength2)(OCISvcCtx      *svchp,
	OCIError       *errhp,
	OCILobLocator  *locp,
	oraub8         *lenp);

typedef void (*pOCICallbackLobRead2)
(
	void         *ctxp,
	CONST void   *bufp,
	oraub8        lenp,
	ub1           piecep,
	void         **changed_bufpp,
	oraub8        *changed_lenp
	);

typedef sword(*pOCILobRead2)(OCISvcCtx          *svchp,
	OCIError           *errhp,
	OCILobLocator      *locp,
	oraub8             *byte_amtp,
	oraub8             *char_amtp,
	oraub8             offset,
	void              *bufp,
	oraub8             bufl,
	ub1                piece,
	void              *ctxp,
	pOCICallbackLobRead2 (cbfp)
	(
		dvoid         *ctxp,
		CONST dvoid   *bufp,
		oraub8        lenp,
		ub1           piecep,
		void         **changed_bufpp,
		oraub8        *changed_lenp
		),
	ub2                csid,
	ub1                csfrm);

typedef sword(*pOCILobFileExists)(OCISvcCtx        *svchp,
	OCIError         *errhp,
	OCILobLocator    *filep,
	boolean          *flag);