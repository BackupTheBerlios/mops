<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title="mirrors list">
	<stripes:layout-component name="contents">
		<h2>Mirrors list</h2>
		<c:choose>
			<c:when test="${actionBean.mirrorsCount == 0}">
				<b>No mirrors available</b>
			</c:when>
			<c:when test="${actionBean.mirrorsCount != 0}">
				<table class="packages-list" cellpadding="3" cellspacing="0" border="0" width="60%">
					<tr class="packages-list">
						<th class="packages-list-header">Name</th>
						<th class="packages-list-header">URL</th>
						<th class="packages-list-header">Type</th>
					</tr>
					<c:forEach items="${actionBean.mirrors}" var="mirror" varStatus="rowstat">
						<tr class="packages-list ${rowstat.count mod 2 == 0 ? 'even' : 'odd' }">
							<td class="packages-list">${mirror.name}</td>
							<td class="packages-list">
								<a href="${mirror.url}">${mirror.url}</a>
							</td>
							<td class="packages-list">${mirror.type}</td>
						</tr>
					</c:forEach>
				</table>
			</c:when>
		</c:choose>
		<br />
		<stripes:link beanclass="ru.rpunet.webmops.web.admin.AddMirrorActionBean">Add new.</stripes:link>
	</stripes:layout-component>
</stripes:layout-render>