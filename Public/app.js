function formatMoney(num) {
    return "$" + Number(num).toFixed(2);
}

// Renders the JSON returned from C++ onto the page
function renderBudget(data) {
    document.getElementById("incomeDisplay").textContent = formatMoney(data.income);
    document.getElementById("spentDisplay").textContent = formatMoney(data.spent);

    const balanceEl = document.getElementById("balanceDisplay");
    balanceEl.textContent = formatMoney(data.balance);
    balanceEl.className = data.balance >= 0 ? "success" : "danger";

    const tbody = document.getElementById("expenseTableBody");
    if (!data.expenses || data.expenses.length === 0) {
        tbody.innerHTML = `<tr><td colspan="2" class="empty">No expenses added yet.</td></tr>`;
        return;
    }

    tbody.innerHTML = data.expenses.map(item => `
        <tr>
            <td>${item.name}</td>
            <td class="right">${formatMoney(item.amount)}</td>
        </tr>
    `).join("");
}

// 1. Load initial state on page open
async function loadState() {
    const res = await fetch("/api/state");
    const data = await res.json();
    renderBudget(data);
}

// 2. Update Income (calls C++ updateIncome)
document.getElementById("incomeForm").addEventListener("submit", async (e) => {
    e.preventDefault();
    const val = document.getElementById("incomeInput").value;
    const res = await fetch(`/api/income?val=${encodeURIComponent(val)}`);
    renderBudget(await res.json());
    e.target.reset();
});

// 3. Add Expense (calls C++ addExpense -> dynamic heap resize)
document.getElementById("addForm").addEventListener("submit", async (e) => {
    e.preventDefault();
    const name = document.getElementById("expenseName").value.trim();
    const amount = document.getElementById("expenseAmount").value;
    const res = await fetch(`/api/add?name=${encodeURIComponent(name)}&amount=${encodeURIComponent(amount)}`);
    renderBudget(await res.json());
    e.target.reset();
});

// 4. Sort Expenses (calls C++ sortExpenses -> Bubble Sort + Template Swap)
document.getElementById("sortBtn").addEventListener("click", async () => {
    const res = await fetch("/api/sort");
    renderBudget(await res.json());
});

// 5. Find & Edit Expense (calls C++ findExpense -> pointer & nullptr check)
document.getElementById("editForm").addEventListener("submit", async (e) => {
    e.preventDefault();
    const name = document.getElementById("editName").value.trim();
    const amount = document.getElementById("editAmount").value;
    const statusEl = document.getElementById("editStatus");

    const res = await fetch(`/api/edit?name=${encodeURIComponent(name)}&amount=${encodeURIComponent(amount)}`);
    if (!res.ok) {
        statusEl.textContent = `C++ returned nullptr: "${name}" not found.`;
        statusEl.style.color = "#f87171";
        return;
    }

    statusEl.textContent = `Updated "${name}" directly via memory pointer!`;
    statusEl.style.color = "#34d399";
    renderBudget(await res.json());
    e.target.reset();
});

// 6. Savings Projector (calls C++ recursive projectSavings)
document.getElementById("projectForm").addEventListener("submit", async (e) => {
    e.preventDefault();
    const balance = document.getElementById("projBalance").value;
    const ratePercent = parseFloat(document.getElementById("projRate").value);
    const rate = ratePercent / 100.0; // Convert 2% -> 0.02 for C++
    const months = document.getElementById("projMonths").value;

    const res = await fetch(`/api/project?balance=${encodeURIComponent(balance)}&rate=${encodeURIComponent(rate)}&months=${encodeURIComponent(months)}`);
    const data = await res.json();
    document.getElementById("projectResult").textContent =
        `Projected after ${months} months: ${formatMoney(data.projected)}`;
});

loadState();