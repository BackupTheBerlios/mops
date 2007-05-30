<%@ include file="/taglib.jsp"%>

<div id="sidebar">
	<div class="sblock">
		<h3>Actions</h3>
		<ul class="methods">
			<li><a href="${pageContext.request.contextPath}/List.action">List packages</a></li>
			<li><a href="${pageContext.request.contextPath}/Search.action">Search</a></li>
			<li><stripes:link beanclass="ru.rpunet.webmops.web.UploadPackageActionBean">Upload</stripes:link>
		</ul>
		<br />
	</div>
</div>
