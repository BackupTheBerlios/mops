<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title=" upload package">
	<stripes:layout-component name="contents">
		<h2>Upload package</h2>
		<br />
		<div id="notice-box">
			Important! Only Slackware packages are supported.
		</div>
		<stripes:form beanclass="ru.rpunet.webmops.web.UploadPackageActionBean" focus="name">
			<stripes:errors />
			Name: <stripes:text name="name" /><br />
			Version: <stripes:text name="version" /><br />
			Arch: <stripes:text name="arch" value="i386" /><br />
			Build: <stripes:text name="build" value="1" /><br/>
			md5: <stripes:text name="md5" value="some shit" /><br/>
			location: <stripes:text name="location" value="./s/" /><br/>
			File: <stripes:file name="packageFile" /><br />
			<stripes:submit name="save" value="Save" />
		</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>