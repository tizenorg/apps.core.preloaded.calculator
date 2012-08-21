# for i18n

SET(POFILES ar.po  de_DE.po  fi.po     hu.po     lt.po     pl.po     sl.po     uk.po bg.po  el_GR.po  fr_FR.po  id.po     lv.po     pt_PT.po  sr.po     vi.po ca.po  en.po     he.po     it_IT.po  ms.po     ro.po     sv.po     zh_CN.po cs.po  en_US.po  hi.po     ja_JP.po  nl_NL.po  ru_RU.po  th.po     zh_HK.po da.po  es_ES.po  hr.po     ko_KR.po  no.po     sk.po     tr_TR.po  zh_TW.po)

SET(MSGFMT "/usr/bin/msgfmt")

FOREACH(pofile ${POFILES})
	SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})
	MESSAGE("PO: ${pofile}")
	GET_FILENAME_COMPONENT(absPofile ${pofile} ABSOLUTE)
	GET_FILENAME_COMPONENT(lang ${absPofile} NAME_WE)
	SET(moFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.mo)
	ADD_CUSTOM_COMMAND(
			OUTPUT ${moFile}
			COMMAND ${MSGFMT} -o ${moFile} ${absPofile}
			DEPENDS ${absPofile}
	)
	INSTALL(FILES ${moFile}
			DESTINATION /opt/apps/org.tizen.calculator/res/locale/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
	SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})
