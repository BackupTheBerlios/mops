<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title=" ${actionBean.pkg.name}">
	<stripes:layout-component name="contents">
		<h2>Info for package ${actionBean.pkg.name}</h2><br />
		Name: ${actionBean.pkg.name}<br />
		Version: ${actionBean.pkg.version }<br />
		Arch: ${actionBean.pkg.arch}<br />
		Short description: ${actionBean.pkg.shortDescription}<br />
		Description:<br />${actionBean.pkg.description}<br />
		File name: ${actionBean.pkg.fileName}<br />
		Location: ${actionBean.pkg.location}<br />
		<a href="${pageContext.request.contextPath}/ShowFiles.action?packageId=${actionBean.pkg.id}">Show files</a>
		(${actionBean.filesCount} total);
	</stripes:layout-component>
</stripes:layout-render>
