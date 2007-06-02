<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title="packages list">
	<stripes:layout-component name="contents">
		<table class="packages-list" cellpadding="3" cellspacing="0" width="100%">
			<tr class="packages-list">
				<th class="packages-list-header">Name</th>
				<th class="packages-list-header">Version</th>
				<th class="packages-list-header">Arch</th>
				<th class="packages-list-header">Info</th>
				<th class="packages-list-header" colspan="2">Actions</th>
			</tr>
			<c:forEach items="${actionBean.packages}" var="pkg" varStatus="rowstat">
				<tr class="packages-list ${rowstat.count mod 2 == 0 ? 'even' : 'odd' }">
					<td class="packages-list">${pkg.name}</td>
					<td class="packages-list">${pkg.version}</td>
					<td class="packages-list">${pkg.arch}</td>
					<td class="packages-list">${pkg.shortDescription}</td>
					<td class="packages-list"><a href="${pageContext.request.contextPath}/PackageInfo.action?id=${pkg.id}">Show</a></td>
					<td class="packages-list"><a href="${pageContext.request.contextPath}/Download.action?id=${pkg.id}">Download</a></td>
				</tr>
			</c:forEach>
		</table>
		<c:when test="${actionBean.pageNum > 1}">
			<stripes:link beanclass="ru.rpunet.webmops.web.ListActionBean">
				<stripes:param name="page" value="${actionBean.currentPage - 1}" />
				Prev
			</stripes:link>
		</c:when>
		<c:when test="${actionBean.totalPages != actionBean.pageNum}">
			<stripes:link beanclass="ru.rpunet.webmops.web.ListActionBean">
				<stripes:param name="page" value="${actionBean.currentPage +1}" />
				Next
			</stripes:link>
		</c:when>
	</stripes:layout-component>
</stripes:layout-render>
