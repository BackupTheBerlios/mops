<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title="file list">
	<stripes:layout-component name="contents">
		<h2>File list for package ${actionBean.pkg.name}</h2>
		<c:choose>
			<c:when test="${actionBean.filesTotalNum == 0}">
				<b>No files.</b>
			</c:when>
			<c:when test="${actionBean.filesTotalNum > 0}">
				<ul>
				<c:forEach items="${actionBean.files}" var="file" varStatus="rowstat">
					<li>${file.name}</li>
				</c:forEach>
				</ul>
			</c:when>
		</c:choose>
	</stripes:layout-component>
</stripes:layout-render>
