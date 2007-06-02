<%@ include file="/taglib.jsp" %>

<stripes:layout-render name="/layout/standart.jsp" title="Upload package">
	<stripes:layout-component name="contents">
		<h2>Upload package</h2>
		<br />
		<stripes:form beanclass="ru.rpunet.webmops.web.UploadPackageActionBean" focus="packageFile">
			<stripes:label for="packageFile" />: <br />
			<stripes:file name="packageFile" /><br />
			<stripes:submit name="uploadPackage" value="Upload" />
		</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>