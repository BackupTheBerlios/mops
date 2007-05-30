<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title="distributions list">
	<stripes:layout-component name="contents">
		<h2>Distributions list</h2>
		<c:choose>
			<c:when test="${actionBean.distCount == 0}">
				<b>No distributions defined.</b>
			</c:when>
			<c:when test="${actionBean.distCount > 0}">
				<table class="packages-list" width="80%" cellpadding="3" cellspacing="0" border="0">
				<tr class="packages-list">
					<th class="packages-list-header">Name</th>
					<th class="packages-list-header">Description</th>
					<th class="packages-list-header">Mirrors</th>
				</tr>
				<c:forEach items="${actionBean.distributions}" var="dist" varStatus="rowstat">
					<tr class="packages-list ${rowstat.count mod 2 == 0 ? 'even' : 'odd' }">
						<td class="packages-list">${dist.name}</td>
						<td class="packages-list">${dist.description}</td>
					</tr>	
				</c:forEach>
				</table>
			</c:when>
		</c:choose>
	</stripes:layout-component>
</stripes:layout-render>