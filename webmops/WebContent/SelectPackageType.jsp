<%@ include file="/taglib.jsp" %>

<stripes:layout-render name="/layout/standart.jsp" title="select package type">
	<stripes:layout-component name="contents">
		<h2>Select package type</h2><br />
		<stripes:form beanclass="ru.rpunet.webmops.web.UploadPackageActionBean">
			<stripes:select name="packageType" value="SLACKWARE">
				<stripes:options-enumeration enum="ru.rpunet.webmops.model.PackagesTypes" />
			</stripes:select>
			<stripes:submit name="selectType" value="Select" />
		</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>