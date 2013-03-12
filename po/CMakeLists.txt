# for i18n

SET(POFILES ar.po  cs.po     en_PH.po  es_US.po  fr_CA.po  he.po  hy.po     ja_JP.po  lt.po  nb.po     pt_BR.po  sk.po  th.po     vi.po     zh_TW.po
az.po  da.po     en.po     et.po     fr_FR.po  hi.po  id.po     ka.po     lv.po  nl_NL.po  pt_PT.po  sl.po  tr_TR.po  zh_CN.po
bg.po  de_DE.po  en_US.po  eu.po     ga.po     hr.po  is.po     kk.po     mk.po  no.po     ro.po     sr.po  uk.po     zh_HK.po
ca.po  el_GR.po  es_ES.po  fi.po     gl.po     hu.po  it_IT.po  ko_KR.po  ms.po  pl.po     ru_RU.po  sv.po  uz.po     zh_SG.po
)

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
			DESTINATION /usr/apps/org.tizen.calculator/res/locale/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
	SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})
